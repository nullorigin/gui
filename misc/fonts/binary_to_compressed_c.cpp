// gui
// (binary_to_compressed_c.cpp)
// Helper tool to turn a file into a C array, if you want to embed font data in
// your source code.

// The data is first compressed with compress() to reduce source code size,
// then encoded in Base85 to fit in a string so we can fit roughly 4 bytes of
// compressed data into 5 bytes of source code (suggested by @mmalex) (If we
// used 32-bit constants it would require take 11 bytes of source code to encode
// 4 bytes, and be endianness dependent) Note that even with compression, the
// output array is likely to be bigger than the binary file.. Load compressed
// TTF fonts with Gui::GetIO().Fonts->AddFontFromMemoryCompressedTTF()

// Build with, e.g:
//   # cl.exe binary_to_compressed_c.cpp
//   # g++ binary_to_compressed_c.cpp
//   # clang++ binary_to_compressed_c.cpp
// Usage:
//   binary_to_compressed_c.exe [-base85] [-nocompress] [-nostatic] <inputfile>
//   <symbolname>
// Usage example:
//   # binary_to_compressed_c.exe myfont.ttf MyFont > myfont.cpp
//   # binary_to_compressed_c.exe -base85 myfont.ttf MyFont > myfont.cpp

#define _CRT_SECURE_NO_WARNINGS
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// compress* from stb.h - declaration
typedef unsigned int uint;
typedef unsigned char uchar;
uint compress(uchar *out, uchar *in, uint len);

static bool binary_to_compressed_c(const char *filename, const char *symbol,
                                   bool use_base85_encoding,
                                   bool use_compression, bool use_static);

int main(int argc, char **argv) {
  if (argc < 3) {
    printf("Syntax: %s [-base85] [-nocompress] [-nostatic] <inputfile> "
           "<symbolname>\n",
           argv[0]);
    return 0;
  }

  int argn = 1;
  bool use_base85_encoding = false;
  bool use_compression = true;
  bool use_static = true;
  while (argn < (argc - 2) && argv[argn][0] == '-') {
    if (strcmp(argv[argn], "-base85") == 0) {
      use_base85_encoding = true;
      argn++;
    } else if (strcmp(argv[argn], "-nocompress") == 0) {
      use_compression = false;
      argn++;
    } else if (strcmp(argv[argn], "-nostatic") == 0) {
      use_static = false;
      argn++;
    } else {
      fprintf(stderr, "Unknown argument: '%s'\n", argv[argn]);
      return 1;
    }
  }

  bool ret =
      binary_to_compressed_c(argv[argn], argv[argn + 1], use_base85_encoding,
                             use_compression, use_static);
  if (!ret)
    fprintf(stderr, "Error opening or reading file: '%s'\n", argv[argn]);
  return ret ? 0 : 1;
}

char Encode85Byte(unsigned int x) {
  x = (x % 85) + 35;
  return (char)((x >= '\\') ? x + 1 : x);
}

bool binary_to_compressed_c(const char *filename, const char *symbol,
                            bool use_base85_encoding, bool use_compression,
                            bool use_static) {
  // Read file
  FILE *f = fopen(filename, "rb");
  if (!f)
    return false;
  int data_sz;
  if (fseek(f, 0, SEEK_END) || (data_sz = (int)ftell(f)) == -1 ||
      fseek(f, 0, SEEK_SET)) {
    fclose(f);
    return false;
  }
  char *data = new char[data_sz + 4];
  if (fread(data, 1, data_sz, f) != (size_t)data_sz) {
    fclose(f);
    delete[] data;
    return false;
  }
  memset((void *)(((char *)data) + data_sz), 0, 4);
  fclose(f);

  // Compress
  int maxlen = data_sz + 512 + (data_sz >> 2) + sizeof(int); // total guess
  char *compressed = use_compression ? new char[maxlen] : data;
  int compressed_sz =
      use_compression ? compress((uchar *)compressed, (uchar *)data, data_sz)
                      : data_sz;
  if (use_compression)
    memset(compressed + compressed_sz, 0, maxlen - compressed_sz);

  // Output as Base85 encoded
  FILE *out = stdout;
  fprintf(out, "// File: '%s' (%d bytes)\n", filename, (int)data_sz);
  fprintf(out, "// Exported using binary_to_compressed_c.cpp\n");
  const char *static_str = use_static ? "static " : "";
  const char *compressed_str = use_compression ? "compressed_" : "";
  if (use_base85_encoding) {
    fprintf(out, "%sconst char %s_%sdata_base85[%d+1] =\n    \"", static_str,
            symbol, compressed_str, (int)((compressed_sz + 3) / 4) * 5);
    char prev_c = 0;
    for (int src_i = 0; src_i < compressed_sz; src_i += 4) {
      // This is made a little more complicated by the fact that ??X sequences
      // are interpreted as trigraphs by old C/C++ compilers. So we need to
      // escape pairs of ??.
      unsigned int d = *(unsigned int *)(compressed + src_i);
      for (unsigned int n5 = 0; n5 < 5; n5++, d /= 85) {
        char c = Encode85Byte(d);
        fprintf(out, (c == '?' && prev_c == '?') ? "\\%c" : "%c", c);
        prev_c = c;
      }
      if ((src_i % 112) == 112 - 4)
        fprintf(out, "\"\n    \"");
    }
    fprintf(out, "\";\n\n");
  } else {
    fprintf(out, "%sconst unsigned int %s_%ssize = %d;\n", static_str, symbol,
            compressed_str, (int)compressed_sz);
    fprintf(out, "%sconst unsigned int %s_%sdata[%d/4] =\n{", static_str,
            symbol, compressed_str, (int)((compressed_sz + 3) / 4) * 4);
    int column = 0;
    for (int i = 0; i < compressed_sz; i += 4) {
      unsigned int d = *(unsigned int *)(compressed + i);
      if ((column++ % 12) == 0)
        fprintf(out, "\n    0x%08x, ", d);
      else
        fprintf(out, "0x%08x, ", d);
    }
    fprintf(out, "\n};\n\n");
  }

  // Cleanup
  delete[] data;
  if (use_compression)
    delete[] compressed;
  return true;
}

// compress* from stb.h - definition

////////////////////           compressor         ///////////////////////

static uint adler32(uint adler32, uchar *buffer, uint buflen) {
  const unsigned long ADLER_MOD = 65521;
  unsigned long s1 = adler32 & 0xffff, s2 = adler32 >> 16;
  unsigned long blocklen, i;

  blocklen = buflen % 5552;
  while (buflen) {
    for (i = 0; i + 7 < blocklen; i += 8) {
      s1 += buffer[0], s2 += s1;
      s1 += buffer[1], s2 += s1;
      s1 += buffer[2], s2 += s1;
      s1 += buffer[3], s2 += s1;
      s1 += buffer[4], s2 += s1;
      s1 += buffer[5], s2 += s1;
      s1 += buffer[6], s2 += s1;
      s1 += buffer[7], s2 += s1;

      buffer += 8;
    }

    for (; i < blocklen; ++i)
      s1 += *buffer++, s2 += s1;

    s1 %= ADLER_MOD, s2 %= ADLER_MOD;
    buflen -= blocklen;
    blocklen = 5552;
  }
  return (s2 << 16) + s1;
}

static unsigned int matchlen(uchar *m1, uchar *m2, uint maxlen) {
  uint i;
  for (i = 0; i < maxlen; ++i)
    if (m1[i] != m2[i])
      return i;
  return i;
}

// simple implementation that just takes the source data in a big block

static uchar *_out;
static FILE *_outfile;
static uint _outbytes;

static void _write(unsigned char v) {
  fputc(v, _outfile);
  ++_outbytes;
}

// #define out(v)    (_out ? *_out++ = (uchar) (v) :
// _write((uchar) (v)))
#define out(v)                                                                 \
  do {                                                                         \
    if (_out)                                                                  \
      *_out++ = (uchar)(v);                                                    \
    else                                                                       \
      _write((uchar)(v));                                                      \
  } while (0)

static void out2(uint v) {
  out(v >> 8);
  out(v);
}
static void out3(uint v) {
  out(v >> 16);
  out(v >> 8);
  out(v);
}
static void out4(uint v) {
  out(v >> 24);
  out(v >> 16);
  out(v >> 8);
  out(v);
}

static void outliterals(uchar *in, int numlit) {
  while (numlit > 65536) {
    outliterals(in, 65536);
    in += 65536;
    numlit -= 65536;
  }

  if (numlit == 0)
    ;
  else if (numlit <= 32)
    out(0x000020 + numlit - 1);
  else if (numlit <= 2048)
    out2(0x000800 + numlit - 1);
  else /*  numlit <= 65536) */
    out3(0x070000 + numlit - 1);

  if (_out) {
    memcpy(_out, in, numlit);
    _out += numlit;
  } else
    fwrite(in, 1, numlit, _outfile);
}

static int _window = 0x40000; // 256K

static int not_crap(int best, int dist) {
  return ((best > 2 && dist <= 0x00100) || (best > 5 && dist <= 0x04000) ||
          (best > 7 && dist <= 0x80000));
}

static uint _hashsize = 32768;

// note that you can play with the hashing functions all you
// want without needing to change the decompressor
#define _hc(q, h, c) (((h) << 7) + ((h) >> 25) + q[c])
#define _hc2(q, h, c, d) (((h) << 14) + ((h) >> 18) + (q[c] << 7) + q[d])
#define _hc3(q, c, d, e) ((q[c] << 14) + (q[d] << 7) + q[e])

static unsigned int _running_adler;

static int compress_chunk(uchar *history, uchar *start, uchar *end, int length,
                          int *pending_literals, uchar **chash, uint mask) {
  (void)history;
  int window = _window;
  uint match_max;
  uchar *lit_start = start - *pending_literals;
  uchar *q = start;

#define SCRAMBLE(h) (((h) + ((h) >> 16)) & mask)

  // stop short of the end so we don't scan off the end doing
  // the hashing; this means we won't compress the last few bytes
  // unless they were part of something longer
  while (q < start + length && q + 12 < end) {
    int m;
    uint h1, h2, h3, h4, h;
    uchar *t;
    int best = 2, dist = 0;

    if (q + 65536 > end)
      match_max = (uint)(end - q);
    else
      match_max = 65536;

#define _nc(b, d) ((d) <= window && ((b) > 9 || not_crap((int)(b), (int)(d))))

#define TRY(t, p) /* avoid retrying a match we already tried */                \
  if (p ? dist != (int)(q - t) : 1)                                            \
    if ((m = matchlen(t, q, match_max)) > best)                                \
      if (_nc(m, q - (t)))                                                     \
  best = m, dist = (int)(q - (t))

    // rather than search for all matches, only try 4 candidate locations,
    // chosen based on 4 different hash functions of different lengths.
    // this strategy is inspired by LZO; hashing is unrolled here using the
    // 'hc' macro
    h = _hc3(q, 0, 1, 2);
    h1 = SCRAMBLE(h);
    t = chash[h1];
    if (t)
      TRY(t, 0);
    h = _hc2(q, h, 3, 4);
    h2 = SCRAMBLE(h);
    h = _hc2(q, h, 5, 6);
    t = chash[h2];
    if (t)
      TRY(t, 1);
    h = _hc2(q, h, 7, 8);
    h3 = SCRAMBLE(h);
    h = _hc2(q, h, 9, 10);
    t = chash[h3];
    if (t)
      TRY(t, 1);
    h = _hc2(q, h, 11, 12);
    h4 = SCRAMBLE(h);
    t = chash[h4];
    if (t)
      TRY(t, 1);

    // because we use a shared hash table, can only update it
    // _after_ we've probed all of them
    chash[h1] = chash[h2] = chash[h3] = chash[h4] = q;

    if (best > 2)
      assert(dist > 0);

    // see if our best match qualifies
    if (best < 3) { // fast path literals
      ++q;
    } else if (best > 2 && best <= 0x80 && dist <= 0x100) {
      outliterals(lit_start, (int)(q - lit_start));
      lit_start = (q += best);
      out(0x80 + best - 1);
      out(dist - 1);
    } else if (best > 5 && best <= 0x100 && dist <= 0x4000) {
      outliterals(lit_start, (int)(q - lit_start));
      lit_start = (q += best);
      out2(0x4000 + dist - 1);
      out(best - 1);
    } else if (best > 7 && best <= 0x100 && dist <= 0x80000) {
      outliterals(lit_start, (int)(q - lit_start));
      lit_start = (q += best);
      out3(0x180000 + dist - 1);
      out(best - 1);
    } else if (best > 8 && best <= 0x10000 && dist <= 0x80000) {
      outliterals(lit_start, (int)(q - lit_start));
      lit_start = (q += best);
      out3(0x100000 + dist - 1);
      out2(best - 1);
    } else if (best > 9 && dist <= 0x1000000) {
      if (best > 65536)
        best = 65536;
      outliterals(lit_start, (int)(q - lit_start));
      lit_start = (q += best);
      if (best <= 0x100) {
        out(0x06);
        out3(dist - 1);
        out(best - 1);
      } else {
        out(0x04);
        out3(dist - 1);
        out2(best - 1);
      }
    } else { // fallback literals if no match was a balanced tradeoff
      ++q;
    }
  }

  // if we didn't get all the way, add the rest to literals
  if (q - start < length)
    q = start + length;

  // the literals are everything from lit_start to q
  *pending_literals = (int)(q - lit_start);

  _running_adler = adler32(_running_adler, start, (uint)(q - start));
  return (int)(q - start);
}

static int compress_inner(uchar *input, uint length) {
  int literals = 0;
  uint len, i;

  uchar **chash;
  chash = (uchar **)malloc(_hashsize * sizeof(uchar *));
  if (chash == nullptr)
    return 0; // failure
  for (i = 0; i < _hashsize; ++i)
    chash[i] = nullptr;

  // stream signature
  out(0x57);
  out(0xbc);
  out2(0);

  out4(0); // 64-bit length requires 32-bit leading 0
  out4(length);
  out4(_window);

  _running_adler = 1;

  len = compress_chunk(input, input, input + length, length, &literals, chash,
                       _hashsize - 1);
  assert(len == length);

  outliterals(input + length - literals, literals);

  free(chash);

  out2(0x05fa); // end opcode

  out4(_running_adler);

  return 1; // success
}

uint compress(uchar *out, uchar *input, uint length) {
  _out = out;
  _outfile = nullptr;

  compress_inner(input, length);

  return (uint)(_out - out);
}
