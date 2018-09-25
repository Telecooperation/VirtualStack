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

SLEEP_BETWEEN_TESTS = 2
TIME_TO_BUILD_NS3_SETUP = 5
EXTRA_TIME_BEFORE_KILL_SECONDS = 180
FILE_ENDING_LIST = ["vsmeta", "vsbar", "vsdelay"]

STACKS_LIST = ["tcpipv4", "sctpipv4", "udpplusipv4"] #"sctpipv4", "udpplusipv4"
TRANSPORT_PROT_LIST = ["sctp", "tcp"]
NS3_TAPPATH = "tappath"
INVS_NAMESPACE = "vsInvs"
RUNTYPE_VS = "vs"
RUNTYPE_RAW = "raw"


class TestcaseConfig:
    programName = "VirtualStack_runner"
    ns3ProgramName = "VirtualStack_ns3"
    directory = os.path.abspath(os.path.join(os.getcwd(), "../bin"))
    cpuLimitCmd = "cpulimit -P %s -l %s" % (os.path.join(directory, programName), "%d")

    def __init__(self):
        pass


class TapNetworkConfig:
    def __init__(self):
        self.inDeviceName = None
        self.outDeviceName = None
        self.tapPathDataRate = "1Gbps"
        self.tapPathLatency = 0
        self.lteModifier = 0

    def getNamespace(self):
        return "vs" + self.outDeviceName


class NS3SetupResult:
    def __init__(self, networkConfig, (inNetwork, inIp, inGateway), (outNetwork, outIp, outGateway), nextFreeIp, namespace):
        self.inNetwork = inNetwork
        self.inIp = inIp
        self.inGateway = inGateway
        self.outNetwork = outNetwork
        self.outIp = outIp
        self.outGateway = outGateway
        self.nextFreeIp = nextFreeIp
        self.namespace = namespace
        self.networkConfig = networkConfig


class NetworkParameter:
    def __init__(self):
        self.ip = ""
        self.network = ""
        self.namespace = ""


class TestcaseParameter:

    def __init__(self):
        self.protocols = [""]
        self.runType = RUNTYPE_VS
        self.bandwidth = [0]
        self.payloadSize = [0]
        self.runtime = 60
        self.retries = 1
        self.name = "final"
        self.inNetwork = NetworkParameter()
        self.invsInNetwork = NetworkParameter()
        self.invsOutNetwork = NetworkParameter()
        self.outNetwork = NetworkParameter()
        self.additionalStacks = ["invalid"]
        self.additionalOverIp = "0"
        self.invsStacks = ["invalid"]
        self.cpuLimit = 0
        self.zipTestcase = True

    def getTestCaseCount(self):
        return len(self.protocols) * len(self.additionalStacks) * len(self.invsStacks) *\
               len(self.bandwidth) * len(self.payloadSize) * self.retries


def lteParameter():
    config = TapNetworkConfig()
    config.inDeviceName = "lteIn"
    config.outDeviceName = "lteOut"
    config.lteModifier = 0.0003
    config.tapPathDataRate = "1000Mbps"
    config.tapPathLatency = 20
    return config


def dslParameter():
    config = TapNetworkConfig()
    config.inDeviceName = "dslIn"
    config.outDeviceName = "dslOut"
    config.tapPathDataRate = "6Mbps"
    config.tapPathLatency = 30
    return config


def backboneParameter():
    config = TapNetworkConfig()
    config.inDeviceName = "backboneIn"
    config.outDeviceName = "backboneOut"
    config.tapPathDataRate = "1Gbps"
    config.tapPathLatency = 0
    return config


# LTE: lteOut -> INVS-Namespace: moveDeviceToNamespace(lteOut, "vsInvs", ...)
# DSL: dslOut -> INVS-Namespace: moveDeviceToNamespace(dslOut, "vsInvs", ...)
# BB: bbOut -> BB-Namespace: moveDeviceToNamespace(bbOut, "vsBB", ...) -> routesToNamespace nicht ins System, sondern in "vsInvs" schieben
# BB: bbIn -> INVS-Namespace: moveDeviceToNamespace(bbIn, "vsInvs", inIp, inGateway, ...) -> -> routesToNamespace ins System, aber mit inGateway=lteInGateway
# sender ohne netns
# invs im invs
# receiver im bb

# Messung: Wenn feste Bandbreite: Welches CPU Limit reicht um zu erfüllen mit gleicher Latenz wie ohne Limit


def main():
    config = TestcaseConfig()

    if raw_input("Delete old vs-Files [N,y]: ") == "y":
        oldFiles = [glob.glob(os.path.join(config.directory, "*." + x)) for x in FILE_ENDING_LIST]
        for files in oldFiles:
            for f in files:
                os.remove(f)

    parameter = TestcaseParameter()
    parameter.protocols = STACKS_LIST
    parameter.invsInNetwork.namespace = INVS_NAMESPACE
    parameter.invsOutNetwork.namespace = INVS_NAMESPACE
    parameter.additionalStacks = STACKS_LIST + ["invalid"]
    parameter.invsStacks = STACKS_LIST
    parameter.cpuLimit = 0
    parameter.runtime = 30
    parameter.retries = 3
    parameter.bandwidth = [25]
    parameter.zipTestcase = True

    #renice ns3
    subprocess.check_output("sudo renice -n -20 -p $(ps x -L | grep %s | grep -v grep | awk '{print $2}')" % config.ns3ProgramName, shell=True)

    #raw_input("Press Enter to exit")
    runTestCase(config, parameter)


def runTestCase(config, parameter):
    if not parameter.name:
        print "Testcase is missing a name, it has been skipped"
        return

    oneRunEstimation = parameter.runtime + SLEEP_BETWEEN_TESTS + TIME_TO_BUILD_NS3_SETUP
    progressPrinter = ProgressPrinter(parameter.getTestCaseCount(), oneRunEstimation)

    print "## Start testcase: %s" % parameter.name
    progressPrinter.printProgress()

    testCaseFiles = []
    for protocol in parameter.protocols:
        for additionalProt in parameter.additionalStacks:
            for invsProt in parameter.invsStacks:
                for bandwidth in parameter.bandwidth:
                    for payloadsize in parameter.payloadSize:
                        lteProc, lteSetupResult, dslProc, dslSetupResult, backboneProc, backboneSetupResult = setupFinalNetwork(config)

                        parameter.inNetwork.ip = lteSetupResult.inIp
                        parameter.inNetwork.network = lteSetupResult.inNetwork
                        parameter.invsInNetwork.ip = lteSetupResult.outIp
                        parameter.invsOutNetwork.ip = backboneSetupResult.inIp
                        parameter.outNetwork.ip = backboneSetupResult.outIp
                        parameter.outNetwork.network = backboneSetupResult.outNetwork
                        parameter.outNetwork.namespace = backboneSetupResult.namespace
                        parameter.additionalOverIp = dslSetupResult.outIp

                        testCaseFiles += _runTestCaseWithRetry(progressPrinter.updateAndPrintProgress, config, parameter,
                                                               protocol, additionalProt, invsProt,
                                                               bandwidth, payloadsize)

                        for x in [(lteProc, lteSetupResult), (dslProc, dslSetupResult), (backboneProc, backboneSetupResult)]:
                            proc = x[0]
                            namespace = x[1].namespace
                            #print "Terminate NS3-Path"
                            proc.communicate("\n")
                            #print "Delete Network Namespace \"%s\"" % namespace
                            if namespace in [x.strip() for x in subprocess.check_output(shlex.split("ip netns list")).splitlines()]:
                                subprocess.check_output(shlex.split("sudo ip netns del %s" % namespace))

    progressPrinter.finishAndPrintProgress(True)
    print "## Build archive of testcases"
    if parameter.zipTestcase:
        zipTestcaseFiles(parameter.name, config.directory, testCaseFiles)
    print "## Done"


def _runTestCaseWithRetry(afterOneRunCallback, config, parameter,
                          protocol, additionalProt, invsProt,
                          bandwidth, payloadsize):
    retValList = []
    for retry in range(0, parameter.retries):
        retVal = _runOneTestCase(config, parameter,
                                 protocol, additionalProt, invsProt,
                                 bandwidth, payloadsize, retry)
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


def _runOneTestCase(config, parameter, protocol, additionalProt, invsProt, bandwidth, payloadsize, retry):
    # print "Starting simulation"
    # $USER is used, so the process is called as non-root
    callingUser = subprocess.check_output("echo $USER", shell=True).strip()
    invsPrefix = "sudo ip netns exec %s sudo -u \"%s\"" % (parameter.invsOutNetwork.namespace, callingUser)
    recvPrefix = "sudo ip netns exec %s sudo -u \"%s\"" % (parameter.outNetwork.namespace, callingUser)

    filenameAdditions = "{prot} {additionalProt} {invsProt} {runtime} {bw} {ps} {retry}"\
        .format(prot=protocol, additionalProt=additionalProt, invsProt=invsProt,
                runtime=parameter.runtime, bw=str(bandwidth), ps=payloadsize, retry=retry)

    sendCmd = "./{prog} vs {prot} route \"{outNetwork},{invsInIp}\" send {bw} {ps} additional {additionalProt} \"{additionalOverIp}\" {inIp} {outIp} {runtime}"\
        .format(prog=config.programName, prot=protocol, outNetwork=parameter.outNetwork.network,
                invsInIp=parameter.invsInNetwork.ip, bw=bandwidth, ps=payloadsize,
                additionalProt=additionalProt, additionalOverIp=parameter.additionalOverIp,
                inIp=parameter.inNetwork.ip,
                outIp=parameter.outNetwork.ip, runtime=parameter.runtime)

    invsCmd = "{netns} ./{prog} invs {routeStack} {invsOutIp} 0.0.0.0"\
        .format(netns=invsPrefix, prog=config.programName,
                routeStack=invsProt,
                invsOutIp=parameter.invsOutNetwork.ip)

    recvCmd = "{netns} ./{prog} vs {prot} route \"{inNetwork},{invsOutIp}\" recv {inIp} {outIp} {runtime} {filenameAdditions}"\
        .format(netns=recvPrefix, prog=config.programName, prot=protocol, inNetwork=parameter.inNetwork.network,
                invsOutIp=parameter.invsOutNetwork.ip, inIp=parameter.inNetwork.ip,
                outIp=parameter.outNetwork.ip, runtime=parameter.runtime, filenameAdditions=filenameAdditions)

    recvProc = subprocess.Popen(shlex.split(recvCmd), cwd=config.directory,
                                    stdout=subprocess.PIPE, stderr=subprocess.PIPE, stdin=subprocess.PIPE)
    invsProc = subprocess.Popen(shlex.split(invsCmd), cwd=config.directory,
                                    stdout=subprocess.PIPE, stderr=subprocess.PIPE, stdin=subprocess.PIPE)

    #$(ps x | grep VirtualStack_runner | grep -v grep | awk '{print $1}')

    time.sleep(1)
    sendProc = subprocess.Popen(shlex.split(sendCmd), cwd=config.directory,
                                  stdout=subprocess.PIPE, stderr=subprocess.PIPE, stdin=subprocess.PIPE)

    #if parameter.cpuLimit > 0:
    #    subprocess.check_output("ps x | grep %s | grep -v grep | awk '{print $1}' | xargs -n1 sudo cpulimit -l %d -b -p" % (config.programName, parameter.cpuLimit), shell=True)

    if parameter.runType == RUNTYPE_RAW:
        handleRunTypeRAW(recvProc, sendProc)

    programTerminated = False
    for i in range(0, parameter.runtime + EXTRA_TIME_BEFORE_KILL_SECONDS):  # wait the runtime + 10seconds to complete
        if recvProc.poll() is not None:
            programTerminated = True
            break
        time.sleep(1)

    if not programTerminated and recvProc.poll() is None:
        recvProc.kill()
        print "Terminated run for: %s" % recvCmd

    recvProcResult, stderr_results = recvProc.communicate()
    #invsProcResult, stderr_results = invsProc.communicate("\n")
    invsProc.stdin.write("\n")

    for i in range(0, 20, 1):  # wait the runtime + 10seconds to complete
        if sendProc.poll() is not None:
            break
        time.sleep(0.1)
    if sendProc.poll() is None:
        sendProc.kill()

    sendProcResult, stderr_results = sendProc.communicate()
    subprocess.call("ps x | grep %s | grep -v grep | awk '{print $1}' | xargs -n1 sudo kill -9" % config.programName, shell=True)

    #print "Sender: ", sendProcResult
    #print "INVS: ", invsProcResult
    #print "Receiver: ", recvProcResult

    # print stdout_results
    retVal = type('', (object,), {"avg": 0, "filenames": []})()
    try:
        lines = recvProcResult.splitlines()
        firstIndexList = [x[0] for x in enumerate(lines) if x[1].startswith("#####")]
        retVal.filenames = [item for sublist in map(lambda x: lines[x + 1].split(":"), firstIndexList) for item in sublist]
        retVal.median = lines[firstIndexList[0] + 2].split("\t")[9] # es ist der median, nicht der AVG
    except:
        print "Median read failed: %s" % recvCmd
        retVal.median = sys.maxint

    return retVal


def setupFinalNetwork(config):
    while True:
        (lteProc, lteSetupResult) = setupNS3Path(config, "0", lteParameter(), INVS_NAMESPACE)
        (dslProc, dslSetupResult) = setupNS3Path(config, lteSetupResult.nextFreeIp, dslParameter(), INVS_NAMESPACE)
        (backboneProc, backboneSetupResult) = setupNS3Path(config, dslSetupResult.nextFreeIp, backboneParameter(),
                                                           withBackRoute=False)
        # Add backroute from vsbackbone to vsInvs
        if moveDeviceToNamespaceWithRoute(backboneSetupResult.networkConfig.inDeviceName, INVS_NAMESPACE,
                                          backboneSetupResult.inIp, backboneSetupResult.inGateway,
                                          backboneSetupResult.outNetwork, lteSetupResult.inGateway,
                                          # lteSetupResult.inGateway damit lässt sich einstellen ob über LTE oder DSL vom sender zum empfänger
                                          backboneSetupResult.outNetwork):
            break
    return lteProc, lteSetupResult, dslProc, dslSetupResult, backboneProc, backboneSetupResult


def setupNS3Path(config, startIp, parameter, customNamespace=None, customInGateway=None, customRouteNamespace=None, withBackRoute=True):
    #print "Create Network Namespace \"%s\"" % parameter.outDeviceName
    setupNamespace = customNamespace or parameter.getNamespace()
    if setupNamespace not in [x.strip() for x in subprocess.check_output(shlex.split("ip netns list")).splitlines()]:
        subprocess.check_output(shlex.split("sudo ip netns add %s" % setupNamespace))

    while True:
        ns3Env = {"DCE_PATH": config.directory, "PATH": config.directory}
        ns3Cmd = "./%s -%s %s %s %s %s %d %s" % (config.ns3ProgramName, NS3_TAPPATH, str(startIp),
                                                 parameter.inDeviceName,
                                                 parameter.outDeviceName,
                                                 parameter.tapPathDataRate,
                                                 parameter.tapPathLatency,
                                                 str(parameter.lteModifier))

        ns3Proc = subprocess.Popen(shlex.split(ns3Cmd), cwd=config.directory, stdin=subprocess.PIPE,
                                   stdout=subprocess.PIPE, stderr=subprocess.PIPE, env=ns3Env)
        inResult = (inNetwork, inIp, inGateway) = ns3Proc.stdout.readline().strip().split(",")
        outResult = (outNetwork, outIp, outGateway) = ns3Proc.stdout.readline().strip().split(",")
        nextFreeIpIndex = ns3Proc.stdout.readline().strip()

        moveResult = moveDeviceToNamespace(parameter.outDeviceName, setupNamespace, outIp, outGateway, inNetwork)

        if moveResult:
            if withBackRoute:
                addRouteToOutNetwork(customInGateway or inGateway, outNetwork, customRouteNamespace)
            break

        stdout_data, stderr_data = ns3Proc.communicate(input='\n')
    time.sleep(1)
    return ns3Proc, NS3SetupResult(parameter, inResult, outResult, nextFreeIpIndex, setupNamespace)


def moveDeviceToNamespaceWithRoute(deviceName, namespace, outIp, outGateway, inNetwork, inGateway, outNetwork, routeNamespace=None):
    if not moveDeviceToNamespace(deviceName, namespace, outIp, outGateway, inNetwork):
        return False

    addRouteToOutNetwork(inGateway, outNetwork, routeNamespace)
    return True


def moveDeviceToNamespace(deviceName, namespace, outIp, outGateway, inNetwork):
    moveDeviceCmd = "sudo ip link set dev {deviceName} netns {namespace} && " \
                    "sudo ip netns exec {namespace} ip link set {deviceName} up && " \
                    "sudo ip netns exec {namespace} ip addr add {outIp}/24 dev {deviceName} && " \
                    "sudo ip netns exec {namespace} ip route add {inNetwork}/24 via {outGateway}"

    cmd = moveDeviceCmd.format(deviceName=deviceName, namespace=namespace, outIp=outIp, outGateway=outGateway, inNetwork=inNetwork)
    # os.system(cmd)
    ipProc = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = ipProc.communicate()
    return ipProc.returncode == 0


def addRouteToOutNetwork(inGateway, outNetwork, namespace=None):
    routesCmd = "ip route add {outNetwork}/24 via {inGateway}".format(
        outNetwork=outNetwork,
        inGateway=inGateway)  # sudo ip route add {innerNetwork}/24 via {senderGateway} &&

    if namespace:
        routesCmd = "ip netns exec {namespace} {cmd}".format(namespace=namespace, cmd=routesCmd)

    val = subprocess.check_output("sudo " + routesCmd, shell=True)


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
