################################################################################
##
## Copyright (C) 2023 Matthew Deutsch
##
## Static is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.
##
## Static is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Static; if not, write to the Free Software
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
##
################################################################################
cmake_minimum_required(VERSION 3.15)
project(nfdext)

add_library(nfdextlib STATIC
    nfdext.cpp
)
target_include_directories(nfdextlib PUBLIC
    ${nativefiledialog_INCLUDE_DIRS}
)
add_dependencies(nfdextlib
    3rdlibnfd
)
target_link_libraries(nfdextlib
    rgmuilib
    ${3rdnativefiledialoglib}
    ${GTK3_LIBRARIES}
)

