# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-
VERSION = '0.1'
APPNAME = 'next-ndnvideo'

from waflib import Build, Logs, Utils, Task, TaskGen, Configure, Options

def options(opt):
    opt.load('compiler_c compiler_cxx gnu_dirs')
    opt.load('boost default-compiler-flags', tooldir=['.waf-tools'])

    opt.add_option('-l', '--log', dest="log", default='', help='log level', action='store')
    opt.add_option('-t', '--test', action='store_true', default=False, dest='with_tests', help='''build unit tests''')

def configure(conf):
    conf.load("compiler_c compiler_cxx gnu_dirs boost default-compiler-flags")

    conf.check_cfg(package='libndn-cxx', args=['--cflags', '--libs'],
                   uselib_store='NDN_CXX', mandatory=True)
		
    conf.check_cfg(package='gstreamer-1.0', args=['--cflags', '--libs'], 
         uselib_store='GSTREAMER', mandatory=True) 

    conf.check_cfg(package='gstreamer-app-1.0', args=['--cflags', '--libs'], 
         uselib_store='GSTREAMERAPP', mandatory=True) 

    conf.env.LIB_PTHREAD = 'pthread'

    USED_BOOST_LIBS = ['system', 'iostreams', 'filesystem', 'random']

    conf.check_boost(lib=USED_BOOST_LIBS, mandatory=True)

    conf.env['WITH_TESTS'] = conf.options.with_tests

    _enable_log(conf) 

def build(bld):

    bld(target="producer",
        features=["cxx", "cxxprogram"],
        source= "src/producer.cpp src/video-generator.cpp src/producer-callback.cpp",
        use='GSTREAMER BOOST NDN_CXX PTHREAD',
        )

    bld(target="repo_producer",
        features=["cxx", "cxxprogram"],
        source= "src/repo_producer.cpp src/video-generator.cpp src/producer-callback.cpp",
        use='GSTREAMER BOOST NDN_CXX PTHREAD',
        )
      
    bld(target="consumer",
        features=["cxx", "cxxprogram"],
        source= "src/consumer.cpp src/video-player.cpp src/consumer-callback.cpp",
        use='GSTREAMER GSTREAMERAPP BOOST NDN_CXX PTHREAD',
        )

    if bld.env['WITH_TESTS']:
        for app in bld.path.ant_glob('test/*.cpp'):
            bld(features=['cxx', 'cxxprogram'],
                source = [app, 'src/consumer-callback.cpp', 'src/video-player.cpp'],
                target = '%s' % (str(app.change_ext('','.cpp'))),
                includes="src", 
                use="GSTREAMER GSTREAMERAPP BOOST NDN_CXX PTHREAD",
                ) 

def _enable_log(conf):
    if Options.options.log == 'trace':
        Logs.pprint("PINK", "Log level set to trace")
        conf.env.append_value("CFLAGS", "-DLOG_LEVEL=6")
        conf.env.append_value("CXXFLAGS", "-DLOG_LEVEL=6")
    elif Options.options.log == 'debug':
        Logs.pprint("PINK", "Log level set to debug")
        conf.env.append_value("CFLAGS", "-DLOG_LEVEL=5")
        conf.env.append_value("CXXFLAGS", "-DLOG_LEVEL=5")
    elif Options.options.log == 'info':
        Logs.pprint("PINK", "Log level set to info")
        conf.env.append_value("CFLAGS", "-DLOG_LEVEL=4")
        conf.env.append_value("CXXFLAGS", "-DLOG_LEVEL=4")
    elif Options.options.log == '':
        pass
    else:
        Logs.pprint("PINK", "unsupported log level")
