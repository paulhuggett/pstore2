INCLUDE(CheckCXXSourceCompiles)

# Sometimes linking against libatomic is required for atomic ops, if
# the platform doesn't support lock-free atomics.

# We modified LLVM's CheckAtomic module and have it check for 64-bit
# atomics.

function(pstore_check_cxx_atomics varname)
  set(OLD_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})
  set(CMAKE_REQUIRED_FLAGS "-std=c++11 ${CMAKE_REQUIRED_FLAGS}")
  check_cxx_source_compiles("
#include <cstdint>
#include <atomic>
std::atomic<std::uint64_t> x (0);

int main() {
  return x.is_lock_free();
}
" ${varname})
  set(CMAKE_REQUIRED_FLAGS ${OLD_CMAKE_REQUIRED_FLAGS})
endfunction(pstore_check_cxx_atomics)

pstore_check_cxx_atomics(PSTORE_HAVE_CXX_ATOMICS_WITHOUT_LIB)

# If not, check if the atomic library works with it.
if(NOT PSTORE_HAVE_CXX_ATOMICS_WITHOUT_LIB)
  list(APPEND CMAKE_REQUIRED_LIBRARIES "atomic")
  pstore_check_cxx_atomics(PSTORE_HAVE_CXX_ATOMICS_WITH_LIB)
  if (NOT PSTORE_HAVE_CXX_ATOMICS_WITH_LIB)
    message(WARNING "Host compiler must support std::atomic!")
  endif()
endif()

