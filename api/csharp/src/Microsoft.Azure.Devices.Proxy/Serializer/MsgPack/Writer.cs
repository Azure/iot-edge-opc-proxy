// ------------------------------------------------------------
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//  Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

namespace Microsoft.Azure.Devices.Proxy.MsgPack {
    using System;
    using System.IO;
    using System.Reflection;
    using System.Text;
    using System.Threading;
    using System.Threading.Tasks;

    public class MsgPackWriter : IDisposable {

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="strm"></param>
        public MsgPackWriter(Stream strm, bool owner = false) {
            _strm = strm;
            _owner = owner;
        }

        /// <summary>
        /// Write nil
        /// </summary>
        /// <param name="ct"></param>
        /// <returns></returns>
        public Task WriteNilAsync(CancellationToken ct) {
            _buffer[0] = (byte)Types.Nil;
            return _strm.WriteAsync(_buffer, 0, 1, ct);
        }

        /// <summary>
        /// Write true or false
        /// </summary>
        /// <param name="val"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public Task WriteAsync(bool val, CancellationToken ct) {
            _buffer[0] = (byte)(val ? Types.True : Types.False);
            return _strm.WriteAsync(_buffer, 0, 1, ct);
        }


        /// <summary>
        /// Write signed byte
        /// </summary>
        /// <param name="val"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public Task WriteAsync(sbyte val, CancellationToken ct) {
            if (val >= 0) {
                return WriteAsync((byte)val, ct);
            }
            else if (val >= -32) {
                _buffer[0] = (byte)((byte)Types.NegativeFixNum | (byte)val);
                return _strm.WriteAsync(_buffer, 0, 1, ct);
            }
            else {
                _buffer[0] = (byte)Types.Int8;
                _buffer[1] = (byte)val;
                return _strm.WriteAsync(_buffer, 0, 2, ct);
            }
        }

        /// <summary>
        /// Write short
        /// </summary>
        /// <param name="val"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public Task WriteAsync(short val, CancellationToken ct) {
            if (val >= 0) {
                return WriteAsync((ushort)val, ct);
            }
            else if (val >= sbyte.MinValue) {
                return WriteAsync((sbyte)val, ct);
            }
            else {
                _buffer[0] = (byte)Types.Int16;
                _buffer[1] = (byte)(val >> 8);
                _buffer[2] = (byte)val;
                return _strm.WriteAsync(_buffer, 0, 3, ct);
            }
        }

        /// <summary>
        /// Write 32 bit signed
        /// </summary>
        /// <param name="val"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public Task WriteAsync(int val, CancellationToken ct) {
            if (val >= 0) {
                return WriteAsync((uint)val, ct);
            }
            else if (val >= short.MinValue) {
                return WriteAsync((short)val, ct);
            }
            else {
                _buffer[0] = (byte)Types.Int32;
                _buffer[1] = (byte)(val >> 24);
                _buffer[2] = (byte)(val >> 16);
                _buffer[3] = (byte)(val >> 8);
                _buffer[4] = (byte)val;
                return _strm.WriteAsync(_buffer, 0, 5, ct);
            }
        }

        /// <summary>
        /// Write 64 bit signed value
        /// </summary>
        /// <param name="val"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public Task WriteAsync(long val, CancellationToken ct) {
            if (val >= 0) {
                return WriteAsync((ulong)val, ct);
            }
            if (val >= int.MinValue) {
                return WriteAsync((int)val, ct);
            }
            else {
                _buffer[0] = (byte)Types.Int64;
                _buffer[1] = (byte)(val >> 56);
                _buffer[2] = (byte)(val >> 48);
                _buffer[3] = (byte)(val >> 40);
                _buffer[4] = (byte)(val >> 32);
                _buffer[5] = (byte)(val >> 24);
                _buffer[6] = (byte)(val >> 16);
                _buffer[7] = (byte)(val >> 8);
                _buffer[8] = (byte)val;
                return _strm.WriteAsync(_buffer, 0, 9, ct);
            }
        }

        /// <summary>
        /// Write a unsigned byte
        /// </summary>
        /// <param name="val"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public Task WriteAsync(byte val, CancellationToken ct) {
            if (val <= sbyte.MaxValue) {
                _buffer[0] = val;
                return _strm.WriteAsync(_buffer, 0, 1, ct);
            }
            else {
                _buffer[0] = (byte)Types.Uint8;
                _buffer[1] = val;
                return _strm.WriteAsync(_buffer, 0, 2, ct);
            }
        }

        /// <summary>
        /// Write unsigned 16 bit short
        /// </summary>
        /// <param name="val"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public Task WriteAsync(ushort val, CancellationToken ct) {
            if (val <= byte.MaxValue) {
                return WriteAsync((byte)val, ct);
            }
            else {
                _buffer[0] = (byte)Types.UInt16;
                _buffer[1] = (byte)(val >> 8);
                _buffer[2] = (byte)val;
                return _strm.WriteAsync(_buffer, 0, 3, ct);
            }
        }

        /// <summary>
        /// Write char
        /// </summary>
        /// <param name="val"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public Task WriteAsync(char val, CancellationToken ct) =>
            WriteAsync((ushort)val, ct);

        /// <summary>
        /// Write unsigned 32 bit int
        /// </summary>
        /// <param name="val"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public Task WriteAsync(uint val, CancellationToken ct) {
            if (val <= ushort.MaxValue) {
                return WriteAsync((ushort)val, ct);
            }
            else {
                _buffer[0] = (byte)Types.UInt32;
                _buffer[1] = (byte)(val >> 24);
                _buffer[2] = (byte)(val >> 16);
                _buffer[3] = (byte)(val >> 8);
                _buffer[4] = (byte)val;
                return _strm.WriteAsync(_buffer, 0, 5, ct);
            }
        }

        /// <summary>
        /// Write unsigned 64 bit long
        /// </summary>
        /// <param name="val"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public Task WriteAsync(ulong val, CancellationToken ct) {
            if (val <= uint.MaxValue) {
                return WriteAsync((uint)val, ct);
            }
            else {
                _buffer[0] = (byte)Types.UInt64;
                _buffer[1] = (byte)(val >> 56);
                _buffer[2] = (byte)(val >> 48);
                _buffer[3] = (byte)(val >> 40);
                _buffer[4] = (byte)(val >> 32);
                _buffer[5] = (byte)(val >> 24);
                _buffer[6] = (byte)(val >> 16);
                _buffer[7] = (byte)(val >> 8);
                _buffer[8] = (byte)val;
                return _strm.WriteAsync(_buffer, 0, 9, ct);
            }
        }

        /// <summary>
        /// Write a single
        /// </summary>
        /// <param name="val"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public Task WriteAsync(float val, CancellationToken ct) {
            byte[] raw = BitConverter.GetBytes(val); 
            _buffer[0] = (byte)Types.Float;
            if (BitConverter.IsLittleEndian) {
                _buffer[1] = raw[3];
                _buffer[2] = raw[2];
                _buffer[3] = raw[1];
                _buffer[4] = raw[0];
            }
            else {
                _buffer[1] = raw[0];
                _buffer[2] = raw[1];
                _buffer[3] = raw[2];
                _buffer[4] = raw[3];
            }
            return _strm.WriteAsync(_buffer, 0, 5, ct);
        }

        /// <summary>
        /// Write a double
        /// </summary>
        /// <param name="val"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public Task WriteAsync(double val, CancellationToken ct) {
            byte[] raw = BitConverter.GetBytes(val); 
            _buffer[0] = (byte)Types.Double;
            if (BitConverter.IsLittleEndian) {
                _buffer[1] = raw[7];
                _buffer[2] = raw[6];
                _buffer[3] = raw[5];
                _buffer[4] = raw[4];
                _buffer[5] = raw[3];
                _buffer[6] = raw[2];
                _buffer[7] = raw[1];
                _buffer[8] = raw[0];
            }
            else {
                _buffer[1] = raw[0];
                _buffer[2] = raw[1];
                _buffer[3] = raw[2];
                _buffer[4] = raw[3];
                _buffer[5] = raw[4];
                _buffer[6] = raw[5];
                _buffer[7] = raw[6];
                _buffer[8] = raw[7];
            }
            return _strm.WriteAsync(_buffer, 0, 9, ct);
        }

        /// <summary>
        /// Write raw buffer header. must be followed by a write of size to
        /// the underlying stream
        /// </summary>
        /// <param name="size"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public Task WriteBinHeaderAsync(int size, CancellationToken ct) {
            if (size <= byte.MaxValue) {
                _buffer[0] = (byte)Types.Bin8;
                _buffer[1] = (byte)size;
                return _strm.WriteAsync(_buffer, 0, 2, ct);
            }
            else if (size <= ushort.MaxValue) {
                _buffer[0] = (byte)Types.Bin16;
                _buffer[1] = (byte)(size >> 8);
                _buffer[2] = (byte)size;
                return _strm.WriteAsync(_buffer, 0, 3, ct);
            }
            else {
                _buffer[0] = (byte)Types.Bin32;
                _buffer[1] = (byte)(size >> 24);
                _buffer[2] = (byte)(size >> 16);
                _buffer[3] = (byte)(size >> 8);
                _buffer[4] = (byte)size;
                return _strm.WriteAsync(_buffer, 0, 5, ct);
            }
        }

        /// <summary>
        /// Write given buffer
        /// </summary>
        /// <param name="bytes"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public async Task WriteAsync(byte[] bytes, CancellationToken ct) {
            await WriteBinHeaderAsync(bytes.Length, ct);
            await _strm.WriteAsync(bytes, 0, bytes.Length, ct);
        }

        /// <summary>
        /// Write array header
        /// </summary>
        /// <param name="size"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public Task WriteArrayHeaderAsync(int size, CancellationToken ct) {
            if (size <= 15) {
                _buffer[0] = (byte)((byte)Types.FixArray | (byte)size);
                return _strm.WriteAsync(_buffer, 0, 1, ct);
            }
            else if (size <= ushort.MaxValue) {
                _buffer[0] = (byte)Types.Array16;
                _buffer[1] = (byte)(size >> 8);
                _buffer[2] = (byte)size;
                return _strm.WriteAsync(_buffer, 0, 3, ct);
            }
            else {
                _buffer[0] = (byte)Types.Array32;
                _buffer[1] = (byte)(size >> 24);
                _buffer[2] = (byte)(size >> 16);
                _buffer[3] = (byte)(size >> 8);
                _buffer[4] = (byte)size;
                return _strm.WriteAsync(_buffer, 0, 5, ct);
            }
        }

        /// <summary>
        /// Write map header
        /// </summary>
        /// <param name="size"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public Task WriteMapHeaderAsync(int size, CancellationToken ct) {
            if (size <= 15) {
                _buffer[0] = (byte)((byte)Types.FixMap | (byte)size);
                return _strm.WriteAsync(_buffer, 0, 1, ct);
            }
            else if (size <= ushort.MaxValue) {
                _buffer[0] = (byte)Types.Map16;
                _buffer[1] = (byte)(size >> 8);
                _buffer[2] = (byte)size;
                return _strm.WriteAsync(_buffer, 0, 3, ct);
            }
            else {
                _buffer[0] = (byte)Types.Map32;
                _buffer[1] = (byte)(size >> 24);
                _buffer[2] = (byte)(size >> 16);
                _buffer[3] = (byte)(size >> 8);
                _buffer[4] = (byte)size;
                return _strm.WriteAsync(_buffer, 0, 5, ct);
            }
        }

        /// <summary>
        /// Write object header - array format is the only supported format.
        /// </summary>
        /// <param name="fields"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public async Task WriteObjectHeaderAsync(int fields, CancellationToken ct) {
            await WriteArrayHeaderAsync(fields + 1, ct);
            await WriteNilAsync(ct);
        }

        /// <summary>
        /// Write string buffer header. must be followed by a write of size to
        /// the underlying stream
        /// </summary>
        /// <param name="size"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public Task WriteStrHeaderAsync(int size, CancellationToken ct) {
            if (size <= 31) {
                _buffer[0] = (byte)((byte)Types.FixStr | (byte)size);
                return _strm.WriteAsync(_buffer, 0, 1, ct);
            }
            else if (size <= byte.MaxValue) {
                _buffer[0] = (byte)Types.Str8;
                _buffer[1] = (byte)size;
                return _strm.WriteAsync(_buffer, 0, 2, ct);
            }
            else if (size <= ushort.MaxValue) {
                _buffer[0] = (byte)Types.Str16;
                _buffer[1] = (byte)(size >> 8);
                _buffer[2] = (byte)size;
                return _strm.WriteAsync(_buffer, 0, 3, ct);
            }
            else {
                _buffer[0] = (byte)Types.Str32;
                _buffer[1] = (byte)(size >> 24);
                _buffer[2] = (byte)(size >> 16);
                _buffer[3] = (byte)(size >> 8);
                _buffer[4] = (byte)size;
                return _strm.WriteAsync(_buffer, 0, 5, ct);
            }
        }

        /// <summary>
        /// Write string
        /// </summary>
        /// <param name="val"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public async Task WriteAsync(string val, CancellationToken ct) {
            char[] chars = val.ToCharArray();
            int len = val.Length;
            await WriteStrHeaderAsync(_encoder.GetByteCount(chars, 0, len, true), ct);

            int copied;
            int offset = 0;
            int encoded;

            bool done = false;
            while (len > 0 || !done) {
                _encoder.Convert(chars, offset, len, _buffer, 0, _buffer.Length, false, 
                    out encoded, out copied, out done);
                await _strm.WriteAsync(_buffer, 0, copied, ct);
                if (done)
                    break;
                len -= encoded;
                offset += encoded;
            }
        }

        /// <summary>
        /// Write one of the message pack types to the stream
        /// </summary>
        /// <param name="val"></param>
        /// <param name="info"></param>
        /// <param name="ct"></param>
        /// <returns></returns>
        public Task WriteAsync(object val, TypeInfo info, CancellationToken ct) =>
            (Task)GetType().GetMethod("WriteAsync", new Type[] { info.AsType(), typeof(CancellationToken) }).
                Invoke(this, new object[] { val, ct });


        public void Dispose() {
            if (_owner) {
                ((IDisposable)_strm).Dispose();
            }
        }

        private Stream _strm;
        private bool _owner;
        private Encoder _encoder = Encoding.UTF8.GetEncoder();
        private byte[] _buffer = new byte[64];
    }
}
