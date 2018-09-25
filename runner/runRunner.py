#!/usr/bin/env python2
# coding=utf-8

import datetime
import glob
import math
import os
import shlex
import subprocess
import sys
import time
import zipfile

SLEEP_BETWEEN_TESTS = 1
EXTRA_TIME_BEFORE_KILL_SECONDS = 180
FILE_ENDING_LIST = ["vsmeta", "vsbar", "vsdelay"]

BW_NS3 = [3, 6, 10, 25, 50, 100, 0]
PS_LIST = [100] + range(200, 1401, 200)
STACKS_LIST = ["tcpipv4", "udpipv4", "udpliteipv4", "sctpipv4", "dccpipv4", "udpplusipv4"]
TRANSPORT_PROT_LIST = ["tcp", "sctp", "udp"]  # ["sctp", "tcp", "udp"]
RUNTYPE_VS = "vs"
RUNTYPE_RAW = "raw"

NS3_TAP = "tap"
NS3_TAPLEX = "taplex"
NS3_TAPPATH = "tappath"
NS3_NAMESPACE = "vsright"


class ProgressPrinter:
    tasksCount = 0
    finishedCounter = 0
    lastRuntime = 0
    secondsPassed = 0

    def __init__(self, tasksCount, oneRunEstimation):
        self.tasksCount = tasksCount
        self.secondsPassed = oneRunEstimation
        self.lastRuntime = datetime.datetime.now()

    def updateAndPrintProgress(self, newLine=False):
        self.updateProgress()
        self.printProgress(newLine)

    def updateProgress(self):
        self.finishedCounter += 1
        self.secondsPassed += (datetime.datetime.now() - self.lastRuntime).seconds
        self.lastRuntime = datetime.datetime.now()

    def finishAndPrintProgress(self, newLine=False):
        self.finishedCounter = self.tasksCount
        self.printProgress(newLine)

    def printProgress(self, newline=False):
        progress = (self.finishedCounter / float(self.tasksCount)) * 100
        timeLeft = datetime.timedelta(seconds=math.ceil(self._getTimeLeft()))
        print '\r[{0}{1}] {2:3}% - left: {3}, Finish at: {4}'.format('#' * int(progress / 10),
                                                                     ' ' * (10 - int(progress / 10)), int(progress),
                                                                     timeLeft,
                                                                     (datetime.datetime.now() + timeLeft).strftime(
                                                                         "%H:%M:%S")),
        sys.stdout.flush()
        if newline:
            print ""

    def _getTimeLeft(self):
        oneRuntime = self.secondsPassed / (self.finishedCounter + 1)  # +1 because of oneTimeEstimation
        return (self.tasksCount - self.finishedCounter) * oneRuntime


class TapNetworkConfig:
    def __init__(self):
        self.tapType = None
        self.startIp = 0
        self.inDeviceName = "left"
        self.outDeviceName = "right"
        self.tapPathDataRate = ["1Gbps"]
        self.tapPathLatency = 0
        self.lteModifier = [0]
        self.lteDataRate = "100Mbps"
        self.lteLatency = 50
        self.dslDataRate = "6Mbps"
        self.dslLatency = 30

    def getTestCaseCount(self):
        return len(self.lteModifier) * len(self.tapPathDataRate)


class TestcaseParameter:
    def __init__(self):
        self.transports = [""]
        self.runType = RUNTYPE_VS
        self.bandwidth = [0]
        self.payloadSize = [0]
        self.runtime = 30
        self.retries = 2
        self.enableDelayDump = False
        self.name = None
        self.zipTestcase = True
        self.networkConfig = TapNetworkConfig()

    def getTestCaseCount(self):
        return len(self.transports) * len(self.bandwidth) * len(self.payloadSize) * self.retries \
               * self.networkConfig.getTestCaseCount()


class TestcaseConfig:
    programName = "VirtualStack_runner"
    ns3ProgramName = "VirtualStack_ns3"
    directory = os.path.abspath(os.path.join(os.getcwd(), "../bin"))
    cpuLimitCmd = "cpulimit -P %s -l %s" % (os.path.join(directory, programName), "%d")

    def __init__(self):
        pass


def main():
    config = TestcaseConfig()

    testCaseList = [
        ns3RawLte(), ns3VSLte(),
        ns3TaplexRaw(), ns3TaplexVS(),
        raw_7a(),
        mouseFlow_7b(), elephantFlow_7b(),
        vs_7c(),
        mouseFlow_7d(), elephantFlow_7d()]

    print "Create Network Namespace \"%s\"" % NS3_NAMESPACE
    if NS3_NAMESPACE not in [x.strip() for x in subprocess.check_output(shlex.split("ip netns list")).splitlines()]:
        subprocess.check_output(shlex.split("sudo ip netns add %s" % NS3_NAMESPACE))

    if raw_input("Delete old vs-Files [N,y]: ") == "y":
        oldFiles = [glob.glob(os.path.join(config.directory, "*." + x)) for x in FILE_ENDING_LIST]
        for files in oldFiles:
            for f in files:
                os.remove(f)

    runAll = False
    for parameter in testCaseList:
        runAll = runAll or raw_input("RunAll [N,y]: ") == "y"
        if runAll or raw_input("%s [N,y]: " % parameter.name) == "y":
            runTestCase(config, parameter)

    print "Delete Network Namespace \"%s\"" % NS3_NAMESPACE
    if NS3_NAMESPACE in [x.strip() for x in subprocess.check_output(shlex.split("ip netns list")).splitlines()]:
        subprocess.check_output(shlex.split("sudo ip netns del %s" % NS3_NAMESPACE))


def ns3RawLte():
    config = TestcaseParameter()
    config.networkConfig.tapType = NS3_TAPPATH
    config.networkConfig.lteModifier = [0.01]
    config.networkConfig.tapPathDataRate = ["25Mbps"]
    config.networkConfig.tapPathLatency = 0
    config.name = "ns3RawLTE"
    config.transports = TRANSPORT_PROT_LIST + ["dccp"]
    config.runType = RUNTYPE_RAW
    config.bandwidth = [25]
    return config


def ns3VSLte():
    config = TestcaseParameter()
    config.networkConfig.tapType = NS3_TAPPATH
    config.networkConfig.lteModifier = [0.01]
    config.networkConfig.tapPathDataRate = ["25Mbps"]
    config.networkConfig.tapPathLatency = 0
    config.name = "ns3VSLTE"
    config.transports = STACKS_LIST
    config.runType = RUNTYPE_VS
    config.bandwidth = [25]
    return config


def ns3TaplexRaw():
    config = TestcaseParameter()
    config.networkConfig.tapType = NS3_TAPLEX
    config.networkConfig.lteModifier = [0.01]
    config.networkConfig.lteDataRate = "25Mbps"
    config.networkConfig.lteLatency = 0
    config.name = "ns3RawTaplex"
    config.transports = ["dccp"] + TRANSPORT_PROT_LIST
    config.runType = RUNTYPE_RAW
    config.bandwidth = [25]
    return config


def ns3TaplexVS():
    config = TestcaseParameter()
    config.networkConfig.tapType = NS3_TAPLEX
    config.networkConfig.lteModifier = [0.01]
    config.networkConfig.lteDataRate = "25Mbps"
    config.networkConfig.lteLatency = 1
    config.networkConfig.tapPathLatency = 1
    config.name = "ns3VSTaplex"
    config.transports = STACKS_LIST
    config.runType = RUNTYPE_VS
    config.bandwidth = [25]
    return config


def raw_7a():
    config = TestcaseParameter()
    config.name = "7a_NS3-Raw"
    config.transports = TRANSPORT_PROT_LIST
    config.runType = RUNTYPE_RAW
    config.bandwidth = BW_NS3
    config.payloadSize = PS_LIST
    config.networkConfig.tapPathDataRate = ["100Gbps"]
    config.networkConfig.tapType = NS3_TAPPATH
    return config


def mouseFlow_7b():
    config = TestcaseParameter()
    config.name = "7b_NS3-RawMouseFlow"
    config.transports = TRANSPORT_PROT_LIST
    config.runType = RUNTYPE_RAW
    config.bandwidth = [float(420) * 8 / 1024 / 1024]
    config.payloadSize = [210]
    config.networkConfig.tapPathDataRate = ["100Gbps"]
    config.networkConfig.tapType = NS3_TAPPATH
    return config


def elephantFlow_7b():
    config = TestcaseParameter()
    config.name = "7b_NS3-RawElephantFlow"
    config.transports = TRANSPORT_PROT_LIST
    config.runType = RUNTYPE_RAW
    config.bandwidth = BW_NS3
    config.networkConfig.tapPathDataRate = ["100Gbps"]
    config.networkConfig.tapType = NS3_TAPPATH
    return config


def vs_7c():
    config = TestcaseParameter()
    config.name = "7c_NS3-VS"
    config.transports = STACKS_LIST
    config.runType = RUNTYPE_VS
    config.bandwidth = BW_NS3
    config.payloadSize = PS_LIST
    config.networkConfig.tapPathDataRate = ["100Gbps"]
    config.networkConfig.tapType = NS3_TAPPATH
    return config


def mouseFlow_7d():
    config = TestcaseParameter()
    config.name = "7d_NS3-VSMouseFlow"
    config.transports = STACKS_LIST
    config.runType = RUNTYPE_VS
    config.bandwidth = [float(420) * 8 / 1024 / 1024]
    config.payloadSize = [210]
    config.networkConfig.tapPathDataRate = ["100Gbps"]
    config.networkConfig.tapType = NS3_TAPPATH
    return config


def elephantFlow_7d():
    config = TestcaseParameter()
    config.name = "7d_NS3-VSElephantFlow"
    config.transports = STACKS_LIST
    config.runType = RUNTYPE_VS
    config.bandwidth = BW_NS3
    config.networkConfig.tapPathDataRate = ["100Gbps"]
    config.networkConfig.tapType = NS3_TAPPATH
    return config


def runTestCase(config, parameter):
    if not parameter.name:
        print "Testcase is missing a name, it has been skipped"
        return

    oneRunEstimation = parameter.runtime + SLEEP_BETWEEN_TESTS
    progressPrinter = ProgressPrinter(parameter.getTestCaseCount(), oneRunEstimation)

    print "## Start testcase: %s" % parameter.name
    progressPrinter.printProgress()

    testCaseFiles = []
    for transport in parameter.transports:
        for payloadSize in parameter.payloadSize:
            for bandwidth in parameter.bandwidth:
                for tapPathBandwidth in parameter.networkConfig.tapPathDataRate:
                    for lteModifier in parameter.networkConfig.lteModifier:
                        caseFiles = _runTestCaseWithRetry(progressPrinter.updateAndPrintProgress, config, parameter, transport,
                                              payloadSize, bandwidth, tapPathBandwidth, lteModifier)
                        testCaseFiles = testCaseFiles + caseFiles

    progressPrinter.finishAndPrintProgress(True)
    print "## Build archive of testcases"
    if parameter.zipTestcase:
        zipTestcaseFiles(parameter.name, config.directory, testCaseFiles)
    print "## Done"


def _runTestCaseWithRetry(afterOneRunCallback, config, parameter, transport, payloadSize, bandwidth, tapPathBandwidth, lteModifier):
    retValList = []
    for retry in range(0, parameter.retries):
        retVal = _runOneTestCase(config, parameter, transport, bandwidth, payloadSize, tapPathBandwidth, lteModifier, retry)
        retValList.append(retVal)
        time.sleep(SLEEP_BETWEEN_TESTS)
        afterOneRunCallback()
    bestrun = retValList.index(min(retValList, key=lambda x: x.median))
    bestrunEntry = retValList.pop(bestrun)
    for val in retValList:
        for name in val.filenames:
            fullName = os.path.join(config.directory, name)
            if os.path.isfile(fullName):
                os.remove(fullName)
    return bestrunEntry.filenames


def _runOneTestCase(config, parameter, transport, bandwidth, payloadSize, tapPathBandwidth, lteModifier, retry):
    ns3Env = {"DCE_PATH": config.directory, "PATH": config.directory}

    ns3Proc, receiverIp, senderIp = setupNS3Tap(config, ns3Env, parameter, tapPathBandwidth, lteModifier)

    filenameParameter = (transport, parameter.runtime, str(bandwidth), payloadSize, tapPathBandwidth, str(lteModifier), retry)
    # print "Starting simulation"
    # $USER is used, so the process is called as non-root
    callingUser = subprocess.check_output("echo $USER", shell=True).strip()
    ns3PrefixForCommand = "sudo ip netns exec %s sudo -u \"%s\" " % (NS3_NAMESPACE, callingUser)
    receiverCmd = "./%s %s %s recv %s %s %d %s %d %s %d %s %s %d" % ((config.programName, parameter.runType, transport,
                                                                     senderIp, receiverIp, parameter.runtime) +
                                                                    filenameParameter)
    senderCmd = "./%s %s %s send %f %d %s %s %d" % (
        config.programName, parameter.runType, transport, bandwidth, payloadSize, senderIp, receiverIp,
        parameter.runtime)

    receiverProc = subprocess.Popen(shlex.split(ns3PrefixForCommand + " " + receiverCmd), cwd=config.directory,
                                    stdout=subprocess.PIPE, stderr=subprocess.PIPE, stdin=subprocess.PIPE)

    time.sleep(1)
    senderProc = subprocess.Popen(shlex.split(senderCmd), cwd=config.directory,
                                  stdout=subprocess.PIPE, stderr=subprocess.PIPE, stdin=subprocess.PIPE)

    if parameter.runType == RUNTYPE_RAW:
        handleRunTypeRAW(receiverProc, senderProc)

    programTerminated = False
    for i in range(0, parameter.runtime + EXTRA_TIME_BEFORE_KILL_SECONDS):  # wait the runtime + 10seconds to complete
        if receiverProc.poll() is not None:
            programTerminated = True
            break
        time.sleep(1)

    if not programTerminated and receiverProc.poll() is None:
        receiverProc.kill()
        print "Terminated run for: %s" % receiverCmd

    stdout_results, stderr_results = receiverProc.communicate()
    stdout_data, stderr_data = ns3Proc.communicate(input='\n')

    for i in range(0, 20, 1):  # wait the runtime + 10seconds to complete
        if senderProc.poll() is not None:
            break
        time.sleep(0.1)
    if senderProc.poll() is None:
        senderProc.kill()

    # print stdout_results
    retVal = type('', (object,), {"avg": 0, "filenames": []})()
    try:
        lines = stdout_results.splitlines()
        firstIndex = [lines.index(x) for x in lines if x.startswith("#####")][0]
        retVal.filenames = lines[firstIndex + 1].split(":")
        retVal.median = lines[firstIndex + 2].split("\t")[9]
    except:
        print "No result for: %s" % receiverCmd
        retVal.median = sys.maxint

    return retVal


def setupNS3Tap(config, ns3Env, parameter, tapPathBandwidth, lteModifier):
    while True:
        ns3Cmd = "./%s -%s %d %s %s %s %d %s %s %d %s %d" % (config.ns3ProgramName, parameter.networkConfig.tapType,
                                                             parameter.networkConfig.startIp,
                                                             parameter.networkConfig.inDeviceName,
                                                             parameter.networkConfig.outDeviceName,
                                                             tapPathBandwidth,
                                                             parameter.networkConfig.tapPathLatency,
                                                             str(lteModifier),
                                                             parameter.networkConfig.lteDataRate,
                                                             parameter.networkConfig.lteLatency,
                                                             parameter.networkConfig.dslDataRate,
                                                             parameter.networkConfig.dslLatency)


        ns3Proc = subprocess.Popen(shlex.split(ns3Cmd), cwd=config.directory, stdin=subprocess.PIPE,
                                   stdout=subprocess.PIPE, stderr=subprocess.PIPE, env=ns3Env)

        (senderNetwork, senderIp, senderGateway) = ns3Proc.stdout.readline().strip().split(",")
        (receiverNetwork, receiverIp, receiverGateway) = ns3Proc.stdout.readline().strip().split(",")

        if not moveDeviceToNamespace(parameter.networkConfig.outDeviceName, NS3_NAMESPACE, senderGateway, receiverNetwork, receiverIp,
                                     receiverGateway):
            stdout_data, stderr_data = ns3Proc.communicate(input='\n')
            time.sleep(1)
        else:
            break
    return ns3Proc, receiverIp, senderIp


def moveDeviceToNamespace(deviceName, namespace, senderGateway, receiverNetwork, receiverIp, receiverGateway):
    routesToNamespace = "sudo ip route add {receiverNetwork}/24 via {senderGateway}".format(
        receiverNetwork=receiverNetwork,
        senderGateway=senderGateway)  # sudo ip route add {innerNetwork}/24 via {senderGateway} &&

    moveDeviceCmd = "sudo ip link set dev {deviceName} netns {namespace} && " \
                    "sudo ip netns exec {namespace} ip link set {deviceName} up && " \
                    "sudo ip netns exec {namespace} ip addr add {receiverIp}/24 dev {deviceName} && " \
                    "sudo ip netns exec {namespace} ip route add default via {receiverGateway}"

    cmd = moveDeviceCmd.format(deviceName=deviceName, namespace=namespace, senderGateway=senderGateway,
                               receiverNetwork=receiverNetwork, receiverIp=receiverIp, receiverGateway=receiverGateway)
    # os.system(cmd)
    ipProc = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = ipProc.communicate()
    if ipProc.returncode != 0:
        return False

    val = subprocess.check_output(routesToNamespace, shell=True)
    return ipProc.returncode == 0


def handleRunTypeRAW(receiverProc, senderProc):
    receiverPort = receiverProc.stdout.readline().strip()
    senderPort = senderProc.stdout.readline().strip()

    receiverProc.stdin.write(senderPort + " \n")
    senderProc.stdin.writelines(receiverPort + "\n")


def zipTestcaseFiles(filename, directory, zipFileList):
    zipFilename = "%s %s.zip" % (filename, datetime.datetime.now().strftime("%y.%m.%d %H_%M"))
    newZip = zipfile.ZipFile(os.path.join(directory, zipFilename), 'w')
    for f in zipFileList:
        fullName = os.path.join(directory, f)
        if os.path.isfile(fullName):
            newZip.write(fullName, f, compress_type=zipfile.ZIP_DEFLATED)
    newZip.close()

    for name in zipFileList:
        fullName = os.path.join(directory, name)
        if os.path.isfile(fullName):
            os.remove(fullName)


if __name__ == "__main__":
    main()
