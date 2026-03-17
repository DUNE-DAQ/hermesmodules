
cmake_minimum_required(VERSION 3.12)
project(hermesmodules VERSION 2.2.4)

find_package(daq-cmake REQUIRED)

daq_setup_environment()


find_package(fmt REQUIRED)
find_package(uhal REQUIRED)
find_package(ers REQUIRED)
find_package(appfwk REQUIRED)
find_package(opmonlib REQUIRED)
find_package(Boost COMPONENTS unit_test_framework REQUIRED)
find_package(confmodel REQUIRED)
find_package(appmodel REQUIRED)

##############################################################################

# See https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/#daq_codegen

daq_protobuf_codegen( opmon/*.proto )


##############################################################################


# See https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/#daq_add_library

daq_add_library(*.cpp LINK_LIBRARIES confmodel::confmodel appmodel::appmodel fmt::fmt ers::ers uhal::uhal)

##############################################################################


# See https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/#daq_add_python_bindings


daq_add_python_bindings(*.cpp) 


##############################################################################


# See https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/#daq_add_plugin

daq_add_plugin(HermesModule duneDAQModule LINK_LIBRARIES appfwk::appfwk ${PROJECT_NAME}) # Replace appfwk library with a more specific library when appropriate

##############################################################################


# See https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/#daq_add_unit_test

#daq_add_unit_test(Placeholder_test LINK_LIBRARIES ${PROJECT_NAME})  # Placeholder_test should be replaced with real unit tests

##############################################################################

daq_install()

