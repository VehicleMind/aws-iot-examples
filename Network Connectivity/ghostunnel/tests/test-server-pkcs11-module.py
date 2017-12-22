#!/usr/bin/env python3

"""
Test that ensures that PKCS11 module support works.
"""

from common import LOCALHOST, STATUS_PORT, TcpClient, print_ok, run_ghostunnel, terminate
import urllib.request
import urllib.error
import urllib.parse
import os
import signal
import json
import sys

if __name__ == "__main__":
    ghostunnel = None
    try:
        # Only run PKCS11 tests if requested
        if 'GHOSTUNNEL_TEST_PKCS11' not in os.environ:
            sys.exit(0)

        # start ghostunnel
        # hack: point target to STATUS_PORT so that /_status doesn't 503.
        ghostunnel = run_ghostunnel(['server',
                                     '--listen={0}:13001'.format(LOCALHOST),
                                     '--target={0}:{1}'.format(LOCALHOST,
                                                               STATUS_PORT),
                                     '--cert=../test-keys/server-cert.pem',
                                     '--pkcs11-module={0}'.format(os.environ['GHOSTUNNEL_TEST_PKCS11_MODULE']),
                                     '--pkcs11-token-label={0}'.format(os.environ['GHOSTUNNEL_TEST_PKCS11_LABEL']),
                                     '--pkcs11-pin={0}'.format(os.environ['GHOSTUNNEL_TEST_PKCS11_PIN']),
                                     '--cacert=../test-keys/cacert.pem',
                                     '--allow-ou=client',
                                     '--status={0}:{1}'.format(LOCALHOST,
                                                               STATUS_PORT)])

        def urlopen(path):
            return urllib.request.urlopen(path, cafile='../test-keys/cacert.pem')

        # block until ghostunnel is up
        TcpClient(STATUS_PORT).connect(3)
        status = json.loads(str(urlopen(
            "https://{0}:{1}/_status".format(LOCALHOST, STATUS_PORT)).read(), 'utf-8'))
        metrics = json.loads(str(urlopen(
            "https://{0}:{1}/_metrics".format(LOCALHOST, STATUS_PORT)).read(), 'utf-8'))

        if not status['ok']:
            raise Exception("ghostunnel reported non-ok status")

        if not isinstance(metrics, list):
            raise Exception("ghostunnel metrics expected to be JSON list")

        # Test reloading
        ghostunnel.send_signal(signal.SIGUSR1)

        status = json.loads(str(urlopen(
            "https://{0}:{1}/_status".format(LOCALHOST, STATUS_PORT)).read(), 'utf-8'))
        if not status['ok']:
            raise Exception("ghostunnel reported non-ok status")

        print_ok("OK")
    finally:
        terminate(ghostunnel)
