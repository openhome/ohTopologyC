import sys
import os
import time

sys.path.append(os.path.join('dependencies', 'AnyPlatform', 'testharness'))

from testharness.servers   import StaticWebServer

rootDir = 'TestScripts'

w = StaticWebServer(rootDir)
w.start()

print
print 'Now hosting [{root_dir}] at {host}:{port}'.format(root_dir = rootDir, host = w.host(), port = w.port())
print

try:
    while True:
        time.sleep(10)
finally:
    w.stop()

