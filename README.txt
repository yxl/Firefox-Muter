/* ***** BEGIN LICENSE BLOCK *****
 * This Source Code is subject to the terms of the Mozilla Public License
 * version 2.0 (the "License"). You can obtain a copy of the License at
 * http://mozilla.org/MPL/2.0/.
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Firefox Muter.
 *
 * The Initial Developer of the Original Code is
 * the Mozilla Online.
 * Portions created by the Initial Developer are Copyright (C) 2011
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *  Yuan Xulei <xyuan@mozilla.com>
 *  Hector Zhao <bzhao@mozilla.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

A Firefox addon allows user to mute the webpage.

Build
==================
Assume the root directory of the project is PROJECT_ROOT.
1. Build Linux share librarys with Ubuntun 32bit
cd PROJECT_ROOT 
sudo apt-get install libtool libpulse-dev
./build-MuterPulseAudio-32.sh
After successful building, you will get libMuterPulseAudio-32.so 
in the directory of 
PROJECT_ROOT\extension\modules\ctypes-binary.

2. Build Linux share librarys with Ubuntun 64bit
cd PROJECT_ROOT 
sudo apt-get install libtool libpulse-dev
./build-MuterPulseAudio-64.sh
After successful building, you will get libMuterPulseAudio-64.so 
in the directory of 
PROJECT_ROOT\extension\modules\ctypes-binary.

3. Build Windows DLL with Visual Studio 2010
Open the solution file: Muter.sln.
Select Menu "Build | Batch Build..." and open "Bath Build" dialog. Check the selection of "MuterHook Release|Win32", "MuterWin7 Release|Win32" and
"MuterWin7 Release|x64". Then build them all. 
After successful building, you will get MuterHook-32.dll, MuterWin7-32.dll
and MuterWin7-64.dll in the directory of 
PROJECT_ROOT\extension\modules\ctypes-binary.

4. Run the following batch file on Windows:
PROJECT_ROOT\tools\buildxpi.bat
You will get muter.xpi in the directory of PROJECT_ROOT.
