/* Copyright (C) 2019-2020 Nemirtingas
   This file is part of the NemirtingasEmuLauncher Launcher

   The NemirtingasEmuLauncher Launcher is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.

   The NemirtingasEmuLauncher Launcher is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the NemirtingasEmuLauncher; if not, see
   <http://www.gnu.org/licenses/>.
 */

using System;
using System.Reflection.Metadata.Ecma335;

namespace NemirtingasEmuLauncher
{
    public class EOSProductId
    {
        private string _Id = string.Empty;
        public string Id
        {
            get => _Id;
            set
            {
                if(!IsValid(value))
                {
                    throw new ArgumentException("Invalid EOS Id, must be a 128 bits max hex string");
                }
                _Id = value;
            }
        }

        public EOSProductId()
        {
        }

        public EOSProductId(string id)
        {
            Id = id;
        }

        public bool IsValid()
        {
            return IsValid(_Id);
        }

        public bool IsValid(string id)
        {
            if (id.Length > 32)
            {
                return false;
            }

            foreach (var c in id)
            {
                if ((c < '0' || c > '9') &&
                   (c < 'a' || c > 'f') &&
                   (c < 'A' || c > 'F'))
                {
                    return false;
                }
            }

            return true;
        }

        public static string RandomId()
        {
            return ((uint)(new Random().NextDouble() * uint.MaxValue)).ToString("x4") +
                   ((uint)(new Random().NextDouble() * uint.MaxValue)).ToString("x4") +
                   ((uint)(new Random().NextDouble() * uint.MaxValue)).ToString("x4") +
                   ((uint)(new Random().NextDouble() * uint.MaxValue)).ToString("x4");
        }

        public static string GenerateIdFromName(string username)
        {
            byte[] eos_id = new byte[16];
            ulong base1 = 0;
            ulong base2 = 0;

            ushort i;

            while (BitConverter.ToUInt64(eos_id, 0) == 0 && BitConverter.ToUInt64(eos_id, 8) == 0)
            {
                eos_id = new byte[16];

                BitConverter.GetBytes(base1).CopyTo(eos_id, 0);
                BitConverter.GetBytes(base2).CopyTo(eos_id, 8);
                
                if ( (base1 + 0x0000001201030307) < base1)
                    base2 += (ulong.MaxValue - base1) + 0x0000001201030307;

                base1 += 0x0000001201030307;

                i = 0;

                foreach (var c in username)
                {
                    eos_id[15 - (i % 16)] ^= (byte)(c + i * 27);
                    eos_id[i % 16] ^= (byte)(c - i * 8);   
                    ++i;
                }
            }

            return BitConverter.ToString(eos_id).ToLower().Replace("-", null);
        }

        public override string ToString()
        {
            return Id;
        }

        public static EOSProductId Parse(string value)
        {
            return new EOSProductId(value);
        }

        public static bool TryParse(string value, out EOSProductId steam_id)
        {
            try
            {
                if (value != null)
                {
                    steam_id = Parse(value);
                    return true;
                }
            }
            catch { }

            steam_id = null;
            return false;
        }
    }
}
