import os
import time


class Metadata:
    def __init__(self, simName, protocol, simTime, bandwidth, payloadSize, sendStackBufferSize, cpuLimit, rawDataPath, quantilePath, zipHandle, lteCoefficient = 0, lteBandwidth = 0, iteration = 0, controlPoint = 0, branch = 0, loss=0, delay=0, vsVariant="simplex", scheduler="pass through", hopName=""):
        self.simName = simName or ""
        self.protocol = protocol or ""
        self.simTime = simTime or ""
        self.bandwidth = bandwidth or ""
        self.payloadSize = payloadSize or ""
        self.sendStackBufferSize = int(sendStackBufferSize or -1) if not isinstance(sendStackBufferSize, float) else sendStackBufferSize #latency coefficient
        self.cpuLimit = cpuLimit or ""
        self.rawDataPath = rawDataPath or ""
        self.quantilePath = quantilePath or ""
        self.quantileData = []
        self.rawData = []
        self.zipHandle = zipHandle or None
        self.lteCoefficient = lteCoefficient
        self.lteBandwidth = lteBandwidth
        self.iteration = iteration
        self.controlPoint = int(controlPoint) if int(controlPoint) != 0 else 9
        self.branch = int(branch)
        self.loss = float(loss)
        self.delay = delay
        self.vsVariant = vsVariant
        self.scheduler = scheduler
        self.hopName = hopName

    def getRawData(self):
        if len(self.rawData) != 0:
            return self.rawData

        start = time.time()
        #print "Reading raw \t\tdata from " + self.rawDataPath
        with self.zipHandle.open(self.rawDataPath) as f:
            content = f.readlines()
        # you may also want to remove whitespace characters like `\n` at the end of each line
        self.rawData = [int(x.strip()) for x in content]
        #print "Finished reading in " + str(time.time() - start)
        return self.rawData

    def getQuantileData(self):
        if len(self.quantileData) != 0:
            #print "Skipping read from " + self.quantilePath
            return self.quantileData

        #print "Reading quantile \tdata from " + self.quantilePath
        with self.zipHandle.open(self.quantilePath) as f:
            content = f.readlines()

        self.quantileData = [max(float(x.strip()), 0) for x in content]
        return self.quantileData

    def parseVsmeta(self, path):
        with self.zipHandle.open(path, "r") as fileHandle:
            self.id, self.testName, self.connectionEstablishmentTime, self.payloadSize, self.packetsSent, self.PPS, self.throughput, self.latAvg, self.latJitter, self.latMedian = fileHandle.readline().strip().split("\t")[:10]
            self.connectionEstablishmentTime = long(self.connectionEstablishmentTime)
            self.payloadSize = long(self.payloadSize)
            self.packetsSent = long(self.packetsSent)
            self.PPS = float(self.PPS)
            self.throughput = float(self.throughput) / 1024.0 / 1024.0
            self.latAvg = float(self.latAvg)
            self.latJitter = float(self.latJitter)
            self.latMedian = long(self.latMedian)

    @staticmethod
    def parseNS3File(path, splt, zipFileHandle):
        pathWithoutExt = os.path.splitext(path)[0]
        return Metadata(splt[0], splt[1].split('-',1)[0].upper(), splt[2], splt[3][:-4], splt[4], None, None, pathWithoutExt + ".vsdelay", pathWithoutExt + ".vsbar", zipFileHandle)

    @staticmethod
    def parseRunnerFile(path, splt, zipFileHandle):
        pathWithoutExt = os.path.splitext(path)[0]
        if splt[1].isdigit() and not "Test" in splt[4]: #11_
            return Metadata(splt[0], (splt[3] + " " + splt[4] + " " + splt[5]).replace("ipv4","").upper(), splt[6], splt[7]+"Mbps", splt[8], None, None, pathWithoutExt + ".vsdelay", pathWithoutExt + ".vsbar", zipFileHandle, iteration=splt[9], controlPoint=splt[1], branch=splt[2])
        if(len(splt) >= 8):
            if "ipv4" in splt[2] or "invalid" in splt[2]:
                return Metadata(splt[0], (splt[1] + " " + splt[2] + " " + splt[3]).replace("ipv4","").upper(), splt[4], splt[5]+"Mbps", splt[6], None, None, pathWithoutExt + ".vsdelay", pathWithoutExt + ".vsbar", zipFileHandle, iteration=splt[7])
            elif "singleHop" in splt[4] or "onePath" in splt[4] or "fullNetwork" in splt[4]:
                protocol = ""
                while "ipv4" in splt[7]:
                    protocol += splt[7]
                    splt = splt[0:7] + splt[8:]
                protocol = protocol.replace("ipv4", "-").upper()[:-1]
                hopName = splt[5]+"->"+splt[6]
                return Metadata(splt[4], protocol, splt[7], splt[8]+"Mbps", splt[9], None, None, pathWithoutExt + ".vsdelay", pathWithoutExt + ".vsbar", zipFileHandle, controlPoint=splt[1], branch=splt[2], vsVariant=splt[12], scheduler=splt[13], hopName=hopName)
            else:
                return Metadata(splt[0], splt[1].replace("ipv4","").upper(), splt[2], splt[3]+"Mbps", splt[4], None, None, pathWithoutExt + ".vsdelay", pathWithoutExt + ".vsbar", zipFileHandle, splt[6], splt[5])
        else:
            return Metadata(splt[0], splt[1].replace("ipv4","").upper(), splt[2], splt[3]+"Mbps", splt[4], None, None, pathWithoutExt + ".vsdelay", pathWithoutExt + ".vsbar", zipFileHandle)

    @staticmethod
    def parseComparingFile(path, splt, zipFileHandle):
        if "AppToApp" in splt[0]:
            simName = "AppToApp"
        elif "Router" in splt[0]:
            simName = "Router"
        elif "Raw" in splt[0]:
            simName = "Raw"
        else:
            print "Unknown sim type " + splt[0]
            return
        protoName = splt[0].replace(simName + " ", "")

        pathWithoutExt = os.path.splitext(path)[0]

        return Metadata(simName, protoName, splt[1], splt[2], splt[3], splt[4],splt[5], pathWithoutExt + ".vsdelay", pathWithoutExt + ".vsbar", zipFileHandle)

    @staticmethod
    def fromFile(path, zipFileHandle):
        #print "Reading meta \t\tdata from " + path
        separator = '_'
        pathWithoutExt = os.path.splitext(path)[0]
        splt = str.split(os.path.basename(pathWithoutExt), separator)

        if "runner" in splt[0]:
            result = Metadata.parseRunnerFile(path, splt, zipFileHandle)
        elif "ns3" in splt[0]:
            result = Metadata.parseNS3File(path, splt, zipFileHandle)
        else:
            result = Metadata.parseComparingFile(path, splt, zipFileHandle)

        result.parseVsmeta(path)
        return result

    @staticmethod
    def getBandwidthKeyFn(max):
        return lambda x: Metadata.bandwidthKeyFn(x, max)

    @staticmethod
    def bandwidthKeyFn(metadata, max=float('Inf')):
        rate = float(metadata.bandwidth.replace("Mbps", ""))
        if rate == 0:
            return max

        return rate

    @staticmethod
    def bandwidthNameFn(metadata, maxValue = 0, maxValueName="no limit"):
        rate = float(metadata.bandwidth.replace("Mbps", ""))
        if(rate == maxValue):
            return maxValueName

        return rate

    @staticmethod
    def getCpuLimitKeyFn(max):
        return lambda x: Metadata.cpuLimitKeyFn(x, max)

    @staticmethod
    def cpuLimitKeyFn(metadata, max=float('Inf')):
        if(metadata.cpuLimit == "0"):
            return max
        return float(metadata.cpuLimit)

    @staticmethod
    def getCpuLimitNameFn(max):
        return lambda x: Metadata.cpuLimitNameFn(x, max)

    @staticmethod
    def cpuLimitNameFn(metadata, max = 0):
        if(int(metadata.cpuLimit) == 0):
            return "no limit"
        return metadata.cpuLimit + "%"

    @staticmethod
    def sendStackBufferSizeKeyFn(metadata):
        if(metadata.sendStackBufferSize == "0"):
            return float('Inf')
        return float(metadata.sendStackBufferSize)