include(CMakeFindDependencyMacro)

find_dependency(uo_linkpool 0.6.0)

include("${CMAKE_CURRENT_LIST_DIR}/winwireTargets.cmake")
