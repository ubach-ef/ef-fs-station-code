#!/usr/bin/env python
import os, sys, traceback
import signal, subprocess, logging
import select
import stat, time, glob
# optparse was new in 2.3, deprecated in 2.7 in favour of new argparse
from optparse import OptionParser

## ATTENTION: need a global variable to hold the subprocess.
logp = None
failures = 0
MAX_FAILURES=3

def die(s="About to die horribly"):
    logging.critical(s)
    stop_logging(logp)
    sys.exit(1)

def finish(*args):
    logging.critical("Caught a SIGTERM: exiting")
    stop_logging(logp)
    sys.exit(0)

def restart(*args):
    logging.warning("Caught a SIGHUP")

def read_config(fn):
    try:
        f = file(fn)
    except IOError:
        die("Failed to open configuration file %s" % fn)
    d = {}
    i = 0
    while True:
        i = i+1
        l = f.readline()
        if l=='': break
        if l.startswith("#"): continue
        if l.endswith("\n"): l = l[:-1]
        try:
            k, v = l.split("=")
        except ValueError:
            die("Can't read configuration file %s, line %d" % (fn, i))
        d[k] = v
    return d
                

def daemonize():
    # a double-fork explanation:
    # http://code.activestate.com/recipes/66012-fork-a-daemon-process-on-unix/#c12
    try:
        pid = os.fork()
    except OSError, r:
        die("Failed to fork"+r)
	pid = None
    if (pid > 0):
        # parent exits
        sys.exit(0)
    try:
        os.close(sys.stdin.fileno())
    except OSError: pass
    try:
        os.close(sys.stdin.fileno())
    except OSError: pass
    try:
        os.close(sys.stderr.fileno())
    except OSError: pass
    ## Add dummies for libraries:
    sys.stdin = open('/dev/null', 'r')
    sys.stdout = open('/dev/null', 'w')
    sys.stderr = open('/dev/null', 'w')

    try:
        os.chdir("/")
    except OSError:
        die("Failed to change directory")
    try:
        os.setsid()
    except OSError:
        die("Failed to setsid")
    os.umask(0)
    logging.debug("About to set signals")
    # * Run in the background.
    signal.signal(signal.SIGINT, signal.SIG_IGN)
    signal.signal(signal.SIGCHLD, signal.SIG_IGN)
    signal.signal(signal.SIGHUP, restart)
    signal.signal(signal.SIGTERM, finish)
    try:
        pid2 = os.fork()
    except OSError, r:
        die("Second fork failed: "+r)
    if pid2 > 0:
        # parent returns
        sys.exit(0)


def get_mk5_ip(mk5fn):
    f = file(mk5fn)
    for l in f.xreadlines():
        if not l.startswith("*"): break
    else:
        die("No mark5 address found in %s" % mk5fn)
    return l.split()[0]
            
def time_since_mod(fn):
    return time.time() - os.stat(fn)[stat.ST_MTIME]

def last_modified(fn_list):
    l = [(time_since_mod(fn), fn) for fn in fn_list]
    l.sort()
    return [(fn, time) for (time, fn) in l]

def stop_logging(logp):
    if logp == None:
        logging.debug("logp is None")
        return
    for p in logp:
        if p.poll() == None:
            logging.debug("Should be killing %d", p.pid)
            try: 
                os.kill(p.pid, signal.SIGTERM)
            except OSError:
                logging.debug("%d already dead!?", p.pid)

def get_recent_logfile(logpath, timeout, poll_interval):
    while True:
        globpath = os.path.join(logpath, "*.log")
        logging.debug("Globbing %s", globpath)
        logs = glob.glob(globpath)
        if not logs:
            logging.debug("No logs: sleeping")
            time.sleep(poll_interval)
            continue
        fn, age = last_modified(logs)[0]
        if age > timeout:
            logging.debug("No recent logs: sleeping")
            time.sleep(poll_interval)
        else: 
            logging.debug("We propose to do something with %s", fn)
            break
    return fn

def main():
    global logp, failures
    parser = OptionParser()
    parser.add_option("-f", "--file", dest="filename",
                      help="write report to FILE", metavar="FILE")
    parser.add_option("-v", "--verbose", help="produce verbose output", 
                      action="store_true", dest="verbose", default=False)
    (options, args) = parser.parse_args()
    if args:
        parser.error("Too many arguments")
    if not options.filename:
        parser.error("No configuration file given")
    config_file = options.filename
    try:
        conf = read_config(config_file)
    except:
        print >>sys.stderr, "Could not read configuration file %s" % config_file
        sys.exit(1)
    local_log = conf.get("LOCALLOG", "./mk5logd.log")
    try:
        if options.verbose:
            level = logging.DEBUG
        else:
            level = logging.INFO
        logging.basicConfig(format='%(asctime)s [%(levelname)s] PID=%(process)d %(message)s', 
                            filename=local_log, level=level)
    except IOError, e:
        print >>sys.stderr, "Could not open logfile %s" % local_log
        sys.exit(1)
    poll_interval = 60 # seconds
    timeout = 15*60 # seconds 
    daemonize()
    try:
        try:
            logging.debug("Daemonized")
            logging.debug("Read config file")
            mk5user = conf.get("MK5USER", "oper")
            mk5secure_s = conf.get("MK5SECURE", "true")
            if mk5secure_s=="true":
                mk5secure = True
            elif mk5secure_s=="false":
                mk5secure = False
            else:
                die("Unknown value for MK5SECURE: "
                    "expected 'true' or 'false'")
            mk5file = conf.get("MK5FILE", "/usr2/control/mk5ad.ctl")
            mk5path = conf.get("MK5PATH", "/home/oper")
            logpath = conf.get("FSLOGPATH", "/usr2/log")
            identfn = conf.get("MK5IDENTITY", "") 
            if identfn!="":
                if not os.path.isfile(identfn):
                    die("Cannot find ssh identity file: %s" % identfn)  
                ssh_ident = "-i %s" % identfn
            else:
               ssh_ident = ""
            logging.debug("About to read %s", mk5file)
            mk5ip = get_mk5_ip(mk5file)
            logging.info("Got Mark5 IP address: %s", mk5ip)
            while True:
                # First we look for a recent log file
                fn = get_recent_logfile(logpath, timeout, poll_interval)
                command1 = ('tail -n +0 -f %s' % fn).split()
                if mk5secure:
                    command2a = ('ssh %s %s@%s' % 
                                 (ssh_ident, mk5user, mk5ip)).split()
                else:
                    # Turn off HostKey checking!
                    # http://linuxcommando.blogspot.com/2008/10/how-to-disable-ssh-host-key-checking.html
                    command2a = ('ssh -q -o StrictHostKeyChecking=no '
                                 '-o UserKnownHostsFile=/dev/null '
                                 '%s %s@%s' % 
                                 (ssh_ident, mk5user, mk5ip)).split()
                command2b = "cat > %s/%s" % (mk5path, os.path.basename(fn))
                ## recipe from python subprocess page:
                command2 = command2a + [command2b]
                logging.debug("Command: %s", str(command2))
                p1 = subprocess.Popen(command1,
                                      stdout=subprocess.PIPE,
                                      stderr=subprocess.PIPE)
                p2 = subprocess.Popen(command2,
                                      stdin=p1.stdout,
                                      stdout=subprocess.PIPE,
                                      stderr=subprocess.PIPE)
                p1.stdout.close()
                logp = [p1, p2]
                poller = select.poll()
                poller.register(p1.stderr)
                poller.register(p2.stderr)
                logging.info("Copying %s to %s:%s/%s", fn, mk5ip, mk5path,
                             os.path.basename(fn))
                while True:
                    if failures > MAX_FAILURES:
                        logging.info("Failed %d times; exiting", failures)
                        sys.exit(3)
                    ret = p1.poll()
                    logging.debug("Tail process poll status: %s", str(ret))
                    errs = poller.poll(0)
                    logging.debug("Stderr poll statuses: %s", str(errs))
                    if ret != None:
                        logging.info("Tail terminated with return code %d", ret)
                        # make sure ssh is dead too
                        stop_logging(logp)
                        break
                    elif errs:
                        logging.info("Unexpected errors in subprocesses")
                        logging.info("Errors: %s", 
                                     ",".join([os.read(n, 255) 
                                               for (n, s) in errs]))
                        stop_logging(logp)
                        failures = failures+1
                        break
                    else:
                        failures=0
                        logging.debug("p2 pid: %d",  p2.pid)
                        logging.debug("status: %s", str(p2.poll()))
                    ## Since we haven't quit, we continue.
                    if time_since_mod(fn) > timeout:
                        ## logp.terminate() introduced in python 2.6;
                        ## we're writing for 2.4
                        stop_logging(logp)
                        logp = None
                        logging.info("Log %s stale; end copying", fn)
                        break
                    else:
                        logging.debug("Not stale yet!")
                        time.sleep(poll_interval)
        ## BaseException introduced in Python2.5; etch has 2.4
        except Exception, e:
            logging.critical("Fatal exception %s\n%s", e, traceback.format_exc())
            stop_logging(logp)
            sys.exit(1)
    except SystemExit, e:        
        logging.critical("Very fatal exception %s\n%s", e,
                         traceback.format_exc())
        stop_logging(logp)
        sys.exit(1)



if __name__=='__main__':
    main()
