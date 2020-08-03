using System;
using System.IO;
using OSUtility;

namespace NemirtingasEmuLauncher
{
    public static class OSFuncs
    {
        public static string GetWindowsEmuApiFolder(bool x64)
        {
            return Path.Combine(Path.GetDirectoryName(Environment.GetCommandLineArgs()[0]), "win" + (x64 ? "64" : "32")) + Path.DirectorySeparatorChar;
        }

        public static string GetLinuxEmuApiFolder(bool x64)
        {
            return Path.Combine(Path.GetDirectoryName(Environment.GetCommandLineArgs()[0]), "linux" + (x64 ? "64" : "32")) + Path.DirectorySeparatorChar;
        }

        public static string GetMacOSEmuApiFolder(bool x64)
        {
            return Path.Combine(Path.GetDirectoryName(Environment.GetCommandLineArgs()[0]), "macosx") + Path.DirectorySeparatorChar;
        }

        public static string GetEmuApiFolder(bool x64)
        {
            if (OSDetector.IsWindows())
            {
                return GetWindowsEmuApiFolder(x64);
            }
            else if (OSDetector.IsLinux())
            {
                return GetLinuxEmuApiFolder(x64);
            }
            else if (OSDetector.IsMacOS())
            {
                return GetMacOSEmuApiFolder(x64);
            }

            return Path.GetDirectoryName(Environment.GetCommandLineArgs()[0]) + Path.DirectorySeparatorChar;
        }

        static public string GetWindowsEpicAPIName(bool x64)
        {
            return x64 ? "EOSSDK-Win64-Shipping.dll" : "EOSSDK-Win32-Shipping.dll";
        }

        static public string GetLinuxEpicAPIName(bool x64)
        {
            return "libEOSSDK-Linux-Shipping.so";
        }

        static public string GetMacOSEpicAPIName(bool x64)
        {
            return "libEOSSDK-Mac-Shipping.dylib";
        }

        static public string GetEpicAPIName(bool x64)
        {
            string game_api = string.Empty;

            if (OSDetector.IsWindows())
            {
                return GetWindowsEpicAPIName(x64);
            }
            else if (OSDetector.IsLinux())
            {
                return GetLinuxEpicAPIName(x64);
            }
            else if (OSDetector.IsMacOS())
            {
                return GetMacOSEpicAPIName(x64);
            }

            return game_api;
        }
    }
}
