# INTEL CONFIDENTIAL
# Copyright (2002-2008) Intel Corporation All Rights Reserved.
# The source code contained or described herein and all documents related to
# the source code ("Material") are owned by Intel Corporation or its suppliers
# or licensors. Title to the Material remains with Intel Corporation or its
# suppliers and licensors. The Material contains trade secrets and proprietary
# and confidential information of Intel or its suppliers and licensors. The
# Material is protected by worldwide copyright and trade secret laws and
# treaty provisions. No part of the Material may be used, copied, reproduced,
# modified, published, uploaded, posted, transmitted, distributed, or
# disclosed in any way without Intel's prior express written permission.
# 
# No license under any patent, copyright, trade secret or other intellectual
# property right is granted to or conferred upon you by disclosure or
# delivery of the Materials, either expressly, by implication, inducement,
# estoppel or otherwise. Any license under such intellectual property rights
# must be express and approved by Intel in writing.


Building on Linux:
	1) Type make in this directory
	2) Output files are:
		./emgdui                    - main executable
		./emgdui.a                  - static library used by the above executable
		./emgdui.so                 - dynamic library used by emgdult

Building on Windows:
	1) Must have Visual Studio 2005 installed on the system
	2) Release version: Run 'build.bat release' (without the quotes)
	3) Output files are:
		./Release/emgdui.exe        - main executable
		./Release/emgdui_static.lib - static library used by above executable
		./Release/emgdui.dll        - dynamic library used by emgdult

	4) Debug version: Run 'build.bat debug' (without the quotes)
	5) Output files are:
		./Debug/emgdui.exe          - main executable
		./Debug/emgdui_static.lib   - static library used by above executable
		./Debug/emgdui.dll          - dynamic library used by emgdult

