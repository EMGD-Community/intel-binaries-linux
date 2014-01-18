#----------------------------------------------------------------------------
# Copyright (c) 2002-2010, Intel Corporation.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#----------------------------------------------------------------------------

DRV     = ch7036
SOURCES = \
            ch7036_port.c \
            ch7036_intf.c \
            ch7036_attr.c \
			ch7036_fw.c \
            ch7036.c \
            ch7036_iic.c \
            ch7036_pm.c \
            ch7036_reg_table.c \
            lvds.c


include ../Makefile.include

#----------------------------------------------------------------------------
# File Revision History
# $Id: Makefile.gnu,v 1.1.2.1 2011/09/13 08:50:22 nanuar Exp $
# $Source: /nfs/fm/proj/eia/cvsroot/koheo/linux.nonredistributable/ch7036/ch7036pd_src/Attic/Makefile.gnu,v $
#----------------------------------------------------------------------------

