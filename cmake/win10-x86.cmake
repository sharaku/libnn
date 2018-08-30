# ----------------------------------------------------------------------------
#
#  MIT License
#  
#  Copyright (c) 2016 Abe Takafumi
#  
#  Permission is hereby granted, free of charge, to any person obtaining a copy
#  of this software and associated documentation files (the "Software"), to deal
#  in the Software without restriction, including without limitation the rights
#  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#  copies of the Software, and to permit persons to whom the Software is
#  furnished to do so, subject to the following conditions:
#  
#  The above copyright notice and this permission notice shall be included in all
#  copies or substantial portions of the Software.
#  
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
#  SOFTWARE. *
#
#
# platfoem: linux
# arch    : x86
#
# ----------------------------------------------------------------------------

set(CMAKE_SYSTEM_NAME Windows)

set(TARGET_OS windows)
set(TARGET_ARCH x86)
set(TARGET_PLATFORM windows)
set(TARGET_SUFFIX "${TARGET_OS}.${TARGET_ARCH}")

set(CROSS_PREFIX )
set(CROSS_TOOLCHAIN_PATH /usr/)
set(CROSS_FLAGS "-Wall -Wno-unused-function")

# ----------------------------------------------------------------------
set(CROSS_FLAGS_C "${CROSS_FLAGS_C} -DSHARAKU_PROF_ENABLE -DSHARAKU_PROF_CLEAR_ENABLE")
set(CROSS_FLAGS_CXX "${CROSS_FLAGS_CXX} -DSHARAKU_PROF_ENABLE -DSHARAKU_PROF_CLEAR_ENABLE")
