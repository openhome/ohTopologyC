#!/usr/bin/python

import sys
import os

from waflib.Node import Node

from wafmodules.filetasks import (
    find_resource_or_fail)

import os.path, sys
sys.path[0:0] = [os.path.join('dependencies', 'AnyPlatform', 'ohWafHelpers')]

from filetasks import gather_files, build_tree, copy_task
from utilfuncs import invoke_test, guess_dest_platform, configure_toolchain, guess_ohnet_location

def options(opt):
    opt.load('msvc')
    opt.load('compiler_cxx')
    opt.load('compiler_c')
    opt.add_option('--ohnet-include-dir', action='store', default=None)
    opt.add_option('--ohnet-lib-dir', action='store', default=None)
    opt.add_option('--testharness-dir', action='store', default=os.path.join('dependencies', 'AnyPlatform', 'testharness'))
    opt.add_option('--ohnet', action='store', default=None)
    opt.add_option('--debug', action='store_const', dest="debugmode", const="Debug", default="Release")
    opt.add_option('--release', action='store_const', dest="debugmode",  const="Release", default="Release")
    opt.add_option('--dest-platform', action='store', default=None)
    opt.add_option('--cross', action='store', default=None)

def configure(conf):

    def set_env(conf, varname, value):
        conf.msg(
                'Setting %s to' % varname,
                "True" if value is True else
                "False" if value is False else
                value)
        setattr(conf.env, varname, value)
        return value

    conf.msg("debugmode:", conf.options.debugmode)
    if conf.options.dest_platform is None:
        try:
            conf.options.dest_platform = guess_dest_platform()
        except KeyError:
            conf.fatal('Specify --dest-platform')

    configure_toolchain(conf)
    guess_ohnet_location(conf)

    conf.env.dest_platform = conf.options.dest_platform
    conf.env.testharness_dir = os.path.abspath(conf.options.testharness_dir)

    if conf.options.dest_platform.startswith('Windows'):
        conf.env.LIB_OHNET=['ws2_32', 'iphlpapi', 'dbghelp']
    conf.env.STLIB_OHNET=['TestFramework', 'ohNetCore']

    if conf.options.dest_platform in ['Core-ppc32', 'Core-armv5', 'Core-armv6']:
        conf.env.append_value('DEFINES', ['DEFINE_TRACE', 'NETWORK_NTOHL_LOCAL', 'NOTERMIOS']) # Tell FLAC to use local ntohl implementation

    conf.env.INCLUDES_OHNETMON = [os.path.join('dependencies', conf.options.dest_platform, 'ohNetmon', 'include')]
    conf.env.STLIBPATH_OHNETMON = [os.path.join(conf.path.find_node('.').abspath(),
                                                os.path.join('dependencies', conf.options.dest_platform, 'ohNetmon', 'lib'))]
    conf.env.STLIB_OHNETMON = ['ohNetmon']

    conf.env.INCLUDES = [
        '.',
        conf.path.find_node('.').abspath()
        ]

    mono = set_env(conf, 'MONO', [] if conf.options.dest_platform.startswith('Windows') else ["mono", "--debug", "--runtime=v4.0"])


    conf.env.STLIB_SHELL = ['Shell']
    

def get_node(bld, node_or_filename):
    if isinstance(node_or_filename, Node):
        return node_or_filename
    return bld.path.find_node(node_or_filename)

def create_copy_task(build_context, files, target_dir='', cwd=None, keep_relative_paths=False, name=None):
    source_file_nodes = [get_node(build_context, f) for f in files]
    if keep_relative_paths:
        cwd_node = build_context.path.find_dir(cwd)
        target_filenames = [
                os.path.join(target_dir, source_node.path_from(cwd_node))
                for source_node in source_file_nodes]
    else:
        target_filenames = [
                os.path.join(target_dir, source_node.name)
                for source_node in source_file_nodes]
        target_filenames = map(build_context.bldnode.make_node, target_filenames)
    return build_context(
            rule=copy_task,
            source=source_file_nodes,
            target=target_filenames,
            name=name)

class GeneratedFile(object):
    def __init__(self, xml, domain, type, version, target):
        self.xml = xml
        self.domain = domain
        self.type = type
        self.version = version
        self.target = target


def build(bld):


    # Library
    bld.stlib(
            source=[
                'OpenHome/WatchableThread.cpp',
                'OpenHome/Watchable.cpp',
                'OpenHome/Command.cpp',
                'OpenHome/DisposeHandler.cpp',
                'OpenHome/Device.cpp',
                'OpenHome/DeviceFactory.cpp',
                'OpenHome/Injector.cpp',
                'OpenHome/Mockable.cpp',
                'OpenHome/Network.cpp',
                'OpenHome/Service.cpp',
                'OpenHome/ServiceProduct.cpp',
                'OpenHome/ServiceReceiver.cpp',
                'OpenHome/ServiceSender.cpp',
                'OpenHome/MetaData.cpp',
                'OpenHome/Topology1.cpp',
                'OpenHome/Topology2.cpp',
                'OpenHome/Topologym.cpp',
            ],
            use=['OHNET'],
            target='ohTopologyC')


    # Tests
    bld.stlib(
           source=[
                'OpenHome/Tests/TestWatchableThread.cpp',
                'OpenHome/Tests/TestWatchable.cpp',
                'OpenHome/Tests/TestShell.cpp',
                'OpenHome/Tests/TestTopology1.cpp',
                'OpenHome/Tests/TestTopology2.cpp',
                'OpenHome/Tests/TestTopologym.cpp',
            ],
            use=['ohTopologyC'],
            target='ohTopologyCTestUtils')

    bld.program(
           source='OpenHome/Tests/TestWatchableThreadMain.cpp',
           use=['OHNET', 'ohTopologyC', 'ohTopologyCTestUtils'],
           target='TestWatchableThread')

    bld.program(
           source='OpenHome/Tests/TestWatchableMain.cpp',
           use=['OHNET', 'ohTopologyC', 'ohTopologyCTestUtils'],
           target='TestWatchable')

    bld.program(
           source='OpenHome/Tests/TestShellMain.cpp',
           use=['OHNET', 'SHELL', 'ohTopologyC', 'ohTopologyCTestUtils'],
           target='TestShell')

    bld.program(
           source='OpenHome/Tests/TestTopology1Main.cpp',
           use=['OHNET', 'ohTopologyC', 'ohTopologyCTestUtils'],
           target='TestTopology1')

    bld.program(
           source='OpenHome/Tests/TestTopology2Main.cpp',
           use=['OHNET', 'ohTopologyC', 'ohTopologyCTestUtils'],
           target='TestTopology2')

    bld.program(
           source='OpenHome/Tests/TestTopologymMain.cpp',
           use=['OHNET', 'ohTopologyC', 'ohTopologyCTestUtils'],
           target='TestTopologym')


# Bundles
def bundle(ctx):
    print 'bundle binaries'
    header_files = gather_files(ctx, '{top}', ['OpenHome/**/*.h'])
    lib_names = ['ohTopologyC']
    lib_files = gather_files(ctx, '{bld}', (ctx.env.cxxstlib_PATTERN % x for x in lib_names))
    bundle_dev_files = build_tree({
        'ohTopologyC/lib' : lib_files,
        'ohTopologyC/include' : header_files
        })
    bundle_dev_files.create_tgz_task(ctx, 'ohTopologyC.tar.gz')

# == Command for invoking unit tests ==

def test(tst):
    if not hasattr(tst, 'test_manifest'):
        tst.test_manifest = 'oncommit.test'
    print 'Testing using manifest:', tst.test_manifest
    rule = 'python {test} -m {manifest} -p {platform} -b {build_dir} -t {tool_dir}'.format(
        test        = os.path.join(tst.env.testharness_dir, 'Test'),
        manifest    = '${SRC}',
        platform    =  tst.env.dest_platform,
        build_dir   = '.',
        tool_dir    = os.path.join('..', 'dependencies', 'AnyPlatform'))
    tst(rule=rule, source=tst.test_manifest)

def test_full(tst):
    tst.test_manifest = 'nightly.test'
    test(tst)

# == Contexts to make 'waf test' work ==

from waflib.Build import BuildContext

class TestContext(BuildContext):
    cmd = 'test'
    fun = 'test'

class TestContext(BuildContext):
    cmd = 'test_full'
    fun = 'test_full'

class BundleContext(BuildContext):
    cmd = 'bundle'
    fun = 'bundle'

# vim: set filetype=python softtabstop=4 expandtab shiftwidth=4 tabstop=4:
