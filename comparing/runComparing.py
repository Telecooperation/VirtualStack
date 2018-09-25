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

SLEEP_AFTER_CPU_LIMIT = 0.5
SLEEP_BETWEEN_TESTS = 2
EXTRA_TIME_BEFORE_KILL_SECONDS = 180
FILE_ENDING_LIST = ["vsmeta", "vsbar", "vsdelay"]

BW_VS = [100, 200, 300, 500, 1000, 0]

APP_TO_APP = range(1, 7)  # appToApp, SoftwareLoop
ROUTER = range(11, 40)   # Router
RAW_PROTCOLS = range(46, 51, 1)
TCP_APPTOAPP_ROUTER_SOFTWARELOOP = [1, 10, 11]  # Tcp, SoftwareLoop, Tcp->Tcp
TCPRAW_VSSTACKS_TCPROUTER = range(1, 7, 1) + [10, 11]

class ProgressPrinter:
    tasksCount = 0
    finishedCounter = 0
    lastRuntime = 0  # datetime.datetime.now() + datetime.timedelta(seconds=calculateTestcaseRuntime(parameter, 1))
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
        print '\r[{0}{1}] {2:3}% - left: {3}, Finish at: {4}'.format('#' * int(progress / 10), ' ' * (10 - int(progress / 10)), int(progress),
                                                                   timeLeft, (datetime.datetime.now() + timeLeft).strftime("%H:%M:%S")),
        sys.stdout.flush()
        if newline:
            print ""

    def _getTimeLeft(self):
        oneRuntime = self.secondsPassed / (self.finishedCounter + 1)  # +1 because of oneTimeEstimation
        return (self.tasksCount - self.finishedCounter) * oneRuntime


class TestcaseParameter:
    testcases = []  # (tcp, softwareloop, tcp->tcp)
    bandwidth = [0]
    sendStackSize = [8]
    cpuLimit = [0]
    payloadSize = [0]
    runtime = 60
    retries = 2
    enableDelayDump = False
    name = None

    def __init__(self):
        pass

    def getTestCaseCount(self):
        return len(self.testcases) * len(self.bandwidth) * len(self.sendStackSize) * len(self.payloadSize) \
               * len(self.cpuLimit) * self.retries


class TestcaseConfig:
    programName = "VirtualStack_comparing"
    directory = os.path.abspath(os.path.join(os.getcwd(), "../bin"))
    cpuLimitCmd = "cpulimit -P %s -l %s" % (os.path.join(directory, programName), "%d")

    def __init__(self):
        pass


def main():
    mtu = getMtu("lo")
    if mtu != 1500:
        print "The lo-interface has to have a MTU of 1500 but was: %d (Hint: sudo ifconfig lo mtu 1500) (Hint2: sudo ip link set " \
              "dev lo mtu 1500)" % mtu
        exit(-1)

    config = TestcaseConfig()

    testCaseList = [appToAppLoopback_1(), rawVariableBW_2(), appToAppMouseFlow_3a(), routerMouseFlow_3a(),
                    appToApp_3b(), router_3b(), appToAppRisingPayloadSize_4(), risingSendStackSize_5(),
                    cpuLimitAppToAppRouterTCP_6()]

    if raw_input("Delete old vs-Files [N,y]: ") == "y":
        oldFiles = [glob.glob(os.path.join(config.directory, "*." + x)) for x in FILE_ENDING_LIST]
        for files in oldFiles:
            for f in files:
                os.remove(f)

    runAll = raw_input("RunAll [N,y]: ") == "y"
    for parameter in testCaseList:
        if runAll or raw_input("%s [N,y]: " % parameter.name) == "y":
            runTestCase(config, parameter)


def appToAppLoopback_1():
    config = TestcaseParameter()
    config.testcases = [10]
    config.bandwidth = BW_VS
    config.name = "1_AppToApp-Loopback"
    return config


def rawVariableBW_2():
    config = TestcaseParameter()
    config.testcases = RAW_PROTCOLS
    config.bandwidth = BW_VS
    config.name = "2_RawWithVariableBW"
    return config


def appToAppMouseFlow_3a():
    config = TestcaseParameter()
    config.testcases = APP_TO_APP
    config.payloadSize = [210]
    config.bandwidth = [float(420) * 8 / 1024 / 1024]
    config.name = "3a_AppToApp-MouseFlow"
    return config


def routerMouseFlow_3a():
    config = TestcaseParameter()
    config.testcases = ROUTER
    config.payloadSize = [210]
    config.bandwidth = [float(420) * 8 / 1024 / 1024]
    config.name = "3a_Router-MouseFlow"
    return config


def appToApp_3b():
    config = TestcaseParameter()
    config.testcases = APP_TO_APP
    config.bandwidth = BW_VS
    config.name = "3b_AppToApp"
    return config


def router_3b():
    config = TestcaseParameter()
    config.testcases = ROUTER
    config.bandwidth = BW_VS
    config.name = "3b_Router"
    return config


def appToAppRisingPayloadSize_4():
    config = TestcaseParameter()
    config.testcases = [10]
    config.payloadSize = range(100, 1401, 100)
    config.name = "4_AppToApp-RisingPayloadSize"
    return config


def risingSendStackSize_5():
    config = TestcaseParameter()
    config.testcases = [1]#TCP_APPTOAPP_ROUTER_SOFTWARELOOP
    config.sendStackSize = [8, 16, 32, 64, 128, 256]
    config.runtime = 30
    config.name = "5_rising-SendStackSize"
    return config


def cpuLimitAppToAppRouterTCP_6():
    config = TestcaseParameter()
    config.testcases = TCPRAW_VSSTACKS_TCPROUTER
    config.bandwidth = [0]
    config.cpuLimit = [0, 10, 50, 100, 150, 200, 300, 400, 500]
    config.runtime = 30
    config.name = "6_CPULimit-AppToApp-Router-TCP"
    return config


#### Jens Paper ####

def appToApp100MBps():
    config = TestcaseParameter()
    config.testcases = APP_TO_APP
    config.bandwidth = [100 * 8]
    config.name = "AppToApp-100Mbps"
    return config


def router100MBps():
    config = TestcaseParameter()
    config.testcases = ROUTER
    config.bandwidth = [100 * 8]
    config.name = "Router-100Mbps"
    return config


def appToAppRouterTCP0to500MBps():
    config = TestcaseParameter()
    config.testcases = TCP_APPTOAPP_ROUTER_SOFTWARELOOP
    config.bandwidth = range(0, 501 * 8, 50 * 8)
    config.name = "AppToApp-Router-TCP-0-500MBps"
    return config


def runTestCase(config, parameter):
    if not parameter.name:
        print "Testcase is missing a name, it has been skipped"
        return

    oneRunEstimation = parameter.runtime + SLEEP_BETWEEN_TESTS + SLEEP_AFTER_CPU_LIMIT
    progressPrinter = ProgressPrinter(parameter.getTestCaseCount(), oneRunEstimation)

    print "## Start testcase: %s" % parameter.name
    progressPrinter.printProgress()

    for testcase in parameter.testcases:
        for payloadSize in parameter.payloadSize:
            for bandwidth in parameter.bandwidth:
                for sendStackSize in parameter.sendStackSize:
                    for cpuLimit in parameter.cpuLimit:
                        _runTestCaseWithRetry(progressPrinter.updateAndPrintProgress, config, parameter, testcase, payloadSize, bandwidth, sendStackSize, cpuLimit)

    progressPrinter.finishAndPrintProgress(True)
    print "## Build archive of testcases"
    zipTestcaseFiles(parameter.name, config.directory)
    print "## Done"


def _runTestCaseWithRetry(afterOneRunCallback, config, parameter, testcase, payloadSize, bandwidth, sendStackSize, cpuLimit):
    retValList = []
    for retry in range(0, parameter.retries):
        if cpuLimit > 0:
            time.sleep(SLEEP_AFTER_CPU_LIMIT)
        retVal = _runOneTestCase(config, parameter, testcase, payloadSize, bandwidth, sendStackSize, cpuLimit, retry)
        retValList.append(retVal)
        time.sleep(SLEEP_BETWEEN_TESTS)
        afterOneRunCallback()
    bestrun = retValList.index(min(retValList, key=lambda x: x.median))
    retValList.pop(bestrun)
    for val in retValList:
        for name in val.filenames:
            fullName = os.path.join(config.directory, name)
            if os.path.isfile(fullName):
                os.remove(fullName)


def _runOneTestCase(config, parameter, testcase, payloadSize, bandwidth, sendStackSize, cpuLimit, retrie):
    # bandwidth shall not be formatted for mosueFlow usability
    comparingCmd = "./%s %d %d %f %d %d %d %d %d" % (config.programName, testcase, parameter.runtime,
                                                       bandwidth, payloadSize,
                                                       sendStackSize, parameter.enableDelayDump,
                                                       cpuLimit, retrie)
    cpuLimitProc = None
    if cpuLimit > 0:
        cpuLimitProc = subprocess.Popen(shlex.split(config.cpuLimitCmd % cpuLimit), stdout=subprocess.PIPE,
                                        stderr=subprocess.PIPE)

    comparingProc = subprocess.Popen(shlex.split(comparingCmd), cwd=config.directory, stdout=subprocess.PIPE,
                                     stderr=subprocess.PIPE)

    programTerminated = False
    for i in range(0, parameter.runtime + EXTRA_TIME_BEFORE_KILL_SECONDS):  # wait the runtime + 10seconds to complete
        if comparingProc.poll() is not None:
            programTerminated = True
            break
        time.sleep(1)

    if not programTerminated:
        comparingProc.kill()
        print "Terminated run for: %s" % comparingCmd

    stdout_results, stderr_results = comparingProc.communicate()
    # print stdout_results

    if cpuLimitProc is not None:
        cpuLimitProc.terminate()

    retVal = type('', (object,), {"avg": 0, "filenames": []})()
    try:
        lines = stdout_results.splitlines()
        firstIndex = [lines.index(x) for x in lines if x.startswith("#####")][0]
        retVal.filenames = lines[firstIndex + 1].split(":")
        retVal.median = lines[firstIndex + 2].split("\t")[9]
    except:
        print "No result for: %s" % comparingCmd
        retVal.median = sys.maxint

    return retVal


def zipTestcaseFiles(filename, directory):
    zipFileList = []
    for file in os.listdir(directory):
        if any(file.endswith(ext) for ext in FILE_ENDING_LIST):
            zipFileList.append(file)

    zipFilename = "%s %s.zip" % (filename, datetime.datetime.now().strftime("%y.%m.%d %H_%M"))
    newZip = zipfile.ZipFile(os.path.join(directory, zipFilename), 'w')
    for file in zipFileList:
        newZip.write(os.path.join(directory, file), file, compress_type=zipfile.ZIP_DEFLATED)
    newZip.close()

    # shutil.make_archive(os.path.join(directory, zipFilename), 'zip', zipFileList)
    for name in zipFileList:
        fullName = os.path.join(directory, name)
        if os.path.isfile(fullName):
            os.remove(fullName)


def getMtu(ifname):
    import socket
    import struct
    from fcntl import ioctl
    SIOCGIFMTU = 0x8921

    '''Use socket ioctl call to get MTU size'''
    s = socket.socket(type=socket.SOCK_DGRAM)
    ifr = ifname + '\x00'*(32-len(ifname))
    try:
        ifs = ioctl(s, SIOCGIFMTU, ifr)
        mtu = struct.unpack('<H',ifs[16:18])[0]
        return mtu
    except Exception, s:
        print 'socket ioctl call failed: {0}'.format(s)
        raise
    finally:
        s.close()


if __name__ == "__main__":
    main()
