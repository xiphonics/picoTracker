/*
Original code by Lee Thomason (www.grinninglizard.com)

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must
not claim that you wrote the original software. If you use this
software in a product, an acknowledgment in the product documentation
would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.
*/

#include "tinyxml2.h"

#include <new> // yes, this one new style header, is in the Android SDK.
#if defined(ANDROID_NDK) || defined(__BORLANDC__) || defined(__QNXNTO__)
#include <stdarg.h>
#include <stddef.h>
#else
#include <cstdarg>
#include <cstddef>
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1400) && (!defined WINCE)
// Microsoft Visual Studio, version 2005 and higher. Not WinCE.
/*int _snprintf_s(
   char *buffer,
   size_t sizeOfBuffer,
   size_t count,
   const char *format [,
          argument] ...
);*/
static inline int TIXML_SNPRINTF(char *buffer, size_t size, const char *format,
                                 ...) {
  va_list va;
  va_start(va, format);
  const int result = vsnprintf_s(buffer, size, _TRUNCATE, format, va);
  va_end(va);
  return result;
}

static inline int TIXML_VSNPRINTF(char *buffer, size_t size, const char *format,
                                  va_list va) {
  const int result = vsnprintf_s(buffer, size, _TRUNCATE, format, va);
  return result;
}

#define TIXML_VSCPRINTF _vscprintf
#define TIXML_SSCANF sscanf_s
#elif defined _MSC_VER
// Microsoft Visual Studio 2003 and earlier or WinCE
#define TIXML_SNPRINTF _snprintf
#define TIXML_VSNPRINTF _vsnprintf
#define TIXML_SSCANF sscanf
#if (_MSC_VER < 1400) && (!defined WINCE)
// Microsoft Visual Studio 2003 and not WinCE.
#define TIXML_VSCPRINTF                                                        \
  _vscprintf // VS2003's C runtime has this, but VC6 C runtime or WinCE SDK
             // doesn't have.
#else
// Microsoft Visual Studio 2003 and earlier or WinCE.
static inline int TIXML_VSCPRINTF(const char *format, va_list va) {
  int len = 512;
  for (;;) {
    len = len * 2;
    char *str = new char[len]();
    const int required = _vsnprintf(str, len, format, va);
    delete[] str;
    if (required != -1) {
      TIXMLASSERT(required >= 0);
      len = required;
      break;
    }
  }
  TIXMLASSERT(len >= 0);
  return len;
}
#endif
#else
// GCC version 3 and higher
// #warning( "Using sn* functions." )
#define TIXML_SNPRINTF snprintf
#define TIXML_VSNPRINTF vsnprintf
static inline int TIXML_VSCPRINTF(const char *format, va_list va) {
  int len = vsnprintf(0, 0, format, va);
  TIXMLASSERT(len >= 0);
  return len;
}
#define TIXML_SSCANF sscanf
#endif

#if defined(_WIN64)
#define TIXML_FSEEK _fseeki64
#define TIXML_FTELL _ftelli64
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) ||    \
    defined(__NetBSD__) || defined(__DragonFly__) || (__CYGWIN__)
#define TIXML_FSEEK fseeko
#define TIXML_FTELL ftello
#elif defined(__ANDROID__)
#if __ANDROID_API__ > 24
#define TIXML_FSEEK fseeko64
#define TIXML_FTELL ftello64
#else
#define TIXML_FSEEK fseeko
#define TIXML_FTELL ftello
#endif
#elif defined(__unix__) && defined(__x86_64__)
#define TIXML_FSEEK fseeko64
#define TIXML_FTELL ftello64
#else
#define TIXML_FSEEK fseek
#define TIXML_FTELL ftell
#endif

static const char LINE_FEED =
    static_cast<char>(0x0a); // all line endings are normalized to LF
static const char LF = LINE_FEED;
static const char CARRIAGE_RETURN =
    static_cast<char>(0x0d); // CR gets filtered out
static const char CR = CARRIAGE_RETURN;
static const char SINGLE_QUOTE = '\'';
static const char DOUBLE_QUOTE = '\"';

// Bunch of unicode info at:
//		http://www.unicode.org/faq/utf_bom.html
//	ef bb bf (Microsoft "lead bytes") - designates UTF-8

static const unsigned char TIXML_UTF_LEAD_0 = 0xefU;
static const unsigned char TIXML_UTF_LEAD_1 = 0xbbU;
static const unsigned char TIXML_UTF_LEAD_2 = 0xbfU;

namespace tinyxml2 {

struct Entity {
  const char *pattern;
  int length;
  char value;
};

static const int NUM_ENTITIES = 5;
static const Entity entities[NUM_ENTITIES] = {{"quot", 4, DOUBLE_QUOTE},
                                              {"amp", 3, '&'},
                                              {"apos", 4, SINGLE_QUOTE},
                                              {"lt", 2, '<'},
                                              {"gt", 2, '>'}};

StrPair::~StrPair() { Reset(); }

void StrPair::TransferTo(StrPair *other) {
  if (this == other) {
    return;
  }
  // This in effect implements the assignment operator by "moving"
  // ownership (as in auto_ptr).

  TIXMLASSERT(other != 0);
  TIXMLASSERT(other->_flags == 0);
  TIXMLASSERT(other->_start == 0);
  TIXMLASSERT(other->_end == 0);

  other->Reset();

  other->_flags = _flags;
  other->_start = _start;
  other->_end = _end;

  _flags = 0;
  _start = 0;
  _end = 0;
}

void StrPair::Reset() {
  if (_flags & NEEDS_DELETE) {
    delete[] _start;
  }
  _flags = 0;
  _start = 0;
  _end = 0;
}

/*
void StrPair::SetStr( const char* str, int flags )
{
    TIXMLASSERT( str );
    Reset();
    size_t len = strlen( str );
    TIXMLASSERT( _start == 0 );
    _start = new char[ len+1 ];
    memcpy( _start, str, len+1 );
    _end = _start + len;
    _flags = flags | NEEDS_DELETE;
}
*/

char *StrPair::ParseText(char *p, const char *endTag, int strFlags,
                         int *curLineNumPtr) {
  TIXMLASSERT(p);
  TIXMLASSERT(endTag && *endTag);
  TIXMLASSERT(curLineNumPtr);

  char *start = p;
  const char endChar = *endTag;
  size_t length = strlen(endTag);

  // Inner loop of text parsing.
  while (*p) {
    if (*p == endChar && strncmp(p, endTag, length) == 0) {
      Set(start, p, strFlags);
      return p + length;
    } else if (*p == '\n') {
      ++(*curLineNumPtr);
    }
    ++p;
    TIXMLASSERT(p);
  }
  return 0;
}

char *StrPair::ParseName(char *p) {
  if (!p || !(*p)) {
    return 0;
  }
  if (!XMLUtil::IsNameStartChar((unsigned char)*p)) {
    return 0;
  }

  char *const start = p;
  ++p;
  while (*p && XMLUtil::IsNameChar((unsigned char)*p)) {
    ++p;
  }

  Set(start, p, 0);
  return p;
}

void StrPair::CollapseWhitespace() {
  // Adjusting _start would cause undefined behavior on delete[]
  TIXMLASSERT((_flags & NEEDS_DELETE) == 0);
  // Trim leading space.
  _start = XMLUtil::SkipWhiteSpace(_start, 0);

  if (*_start) {
    const char *p = _start; // the read pointer
    char *q = _start;       // the write pointer

    while (*p) {
      if (XMLUtil::IsWhiteSpace(*p)) {
        p = XMLUtil::SkipWhiteSpace(p, 0);
        if (*p == 0) {
          break; // don't write to q; this trims the trailing space.
        }
        *q = ' ';
        ++q;
      }
      *q = *p;
      ++q;
      ++p;
    }
    *q = 0;
  }
}

const char *StrPair::GetStr() {
  TIXMLASSERT(_start);
  TIXMLASSERT(_end);
  if (_flags & NEEDS_FLUSH) {
    *_end = 0;
    _flags ^= NEEDS_FLUSH;

    if (_flags) {
      const char *p = _start; // the read pointer
      char *q = _start;       // the write pointer

      while (p < _end) {
        if ((_flags & NEEDS_NEWLINE_NORMALIZATION) && *p == CR) {
          // CR-LF pair becomes LF
          // CR alone becomes LF
          // LF-CR becomes LF
          if (*(p + 1) == LF) {
            p += 2;
          } else {
            ++p;
          }
          *q = LF;
          ++q;
        } else if ((_flags & NEEDS_NEWLINE_NORMALIZATION) && *p == LF) {
          if (*(p + 1) == CR) {
            p += 2;
          } else {
            ++p;
          }
          *q = LF;
          ++q;
        } else if ((_flags & NEEDS_ENTITY_PROCESSING) && *p == '&') {
          // Entities handled by tinyXML2:
          // - special entities in the entity table [in/out]
          // - numeric character reference [in]
          //   &#20013; or &#x4e2d;

          if (*(p + 1) == '#') {
            const int buflen = 10;
            char buf[buflen] = {0};
            int len = 0;
            const char *adjusted =
                const_cast<char *>(XMLUtil::GetCharacterRef(p, buf, &len));
            if (adjusted == 0) {
              *q = *p;
              ++p;
              ++q;
            } else {
              TIXMLASSERT(0 <= len && len <= buflen);
              TIXMLASSERT(q + len <= adjusted);
              p = adjusted;
              memcpy(q, buf, len);
              q += len;
            }
          } else {
            bool entityFound = false;
            for (int i = 0; i < NUM_ENTITIES; ++i) {
              const Entity &entity = entities[i];
              if (strncmp(p + 1, entity.pattern, entity.length) == 0 &&
                  *(p + entity.length + 1) == ';') {
                // Found an entity - convert.
                *q = entity.value;
                ++q;
                p += entity.length + 2;
                entityFound = true;
                break;
              }
            }
            if (!entityFound) {
              // fixme: treat unknown entities as errors?
              *q = *p;
              ++p;
              ++q;
            }
          }
        } else {
          *q = *p;
          ++p;
          ++q;
        }
      }
      TIXMLASSERT(q <= _end);
      _flags = (_flags & NEEDS_DELETE);
      *_end = 0;
    }
    if (_flags & NEEDS_WHITESPACE_COLLAPSING) {
      CollapseWhitespace();
    }
    _flags = (_flags & NEEDS_DELETE);
  }
  TIXMLASSERT(_start);
  return _start;
}

const char *XMLNode::Value() const {
  // Edge case: XMLDocuments don't have a Value. Return null.
  if (this->ToDocument())
    return 0;
  return _value.GetStr();
}
/*
void XMLNode::SetValue( const char* str, bool staticMem )
{
    if ( staticMem ) {
        _value.SetInternedStr( str );
    }
    else {
        _value.SetStr( str );
    }
}
*/

// --------- XMLAttribute ---------- //

const char *XMLAttribute::Name() const { return _name.GetStr(); }

const char *XMLAttribute::Value() const { return _value.GetStr(); }

// --------- XMLUtil ----------- //

const char *XMLUtil::writeBoolTrue = "true";
const char *XMLUtil::writeBoolFalse = "false";

void XMLUtil::SetBoolSerialization(const char *writeTrue,
                                   const char *writeFalse) {
  static const char *defTrue = "true";
  static const char *defFalse = "false";

  writeBoolTrue = (writeTrue) ? writeTrue : defTrue;
  writeBoolFalse = (writeFalse) ? writeFalse : defFalse;
}

const char *XMLUtil::ReadBOM(const char *p, bool *bom) {
  TIXMLASSERT(p);
  TIXMLASSERT(bom);
  *bom = false;
  const unsigned char *pu = reinterpret_cast<const unsigned char *>(p);
  // Check for BOM:
  if (*(pu + 0) == TIXML_UTF_LEAD_0 && *(pu + 1) == TIXML_UTF_LEAD_1 &&
      *(pu + 2) == TIXML_UTF_LEAD_2) {
    *bom = true;
    p += 3;
  }
  TIXMLASSERT(p);
  return p;
}

void XMLUtil::ConvertUTF32ToUTF8(unsigned long input, char *output,
                                 int *length) {
  const unsigned long BYTE_MASK = 0xBF;
  const unsigned long BYTE_MARK = 0x80;
  const unsigned long FIRST_BYTE_MARK[7] = {0x00, 0x00, 0xC0, 0xE0,
                                            0xF0, 0xF8, 0xFC};

  if (input < 0x80) {
    *length = 1;
  } else if (input < 0x800) {
    *length = 2;
  } else if (input < 0x10000) {
    *length = 3;
  } else if (input < 0x200000) {
    *length = 4;
  } else {
    *length = 0; // This code won't convert this correctly anyway.
    return;
  }

  output += *length;

  // Scary scary fall throughs are annotated with carefully designed comments
  // to suppress compiler warnings such as -Wimplicit-fallthrough in gcc
  switch (*length) {
  case 4:
    --output;
    *output = static_cast<char>((input | BYTE_MARK) & BYTE_MASK);
    input >>= 6;
    // fall through
  case 3:
    --output;
    *output = static_cast<char>((input | BYTE_MARK) & BYTE_MASK);
    input >>= 6;
    // fall through
  case 2:
    --output;
    *output = static_cast<char>((input | BYTE_MARK) & BYTE_MASK);
    input >>= 6;
    // fall through
  case 1:
    --output;
    *output = static_cast<char>(input | FIRST_BYTE_MARK[*length]);
    break;
  default:
    TIXMLASSERT(false);
  }
}

const char *XMLUtil::GetCharacterRef(const char *p, char *value, int *length) {
  // Presume an entity, and pull it out.
  *length = 0;

  if (*(p + 1) == '#' && *(p + 2)) {
    unsigned long ucs = 0;
    TIXMLASSERT(sizeof(ucs) >= 4);
    ptrdiff_t delta = 0;
    unsigned mult = 1;
    static const char SEMICOLON = ';';

    if (*(p + 2) == 'x') {
      // Hexadecimal.
      const char *q = p + 3;
      if (!(*q)) {
        return 0;
      }

      q = strchr(q, SEMICOLON);

      if (!q) {
        return 0;
      }
      TIXMLASSERT(*q == SEMICOLON);

      delta = q - p;
      --q;

      while (*q != 'x') {
        unsigned int digit = 0;

        if (*q >= '0' && *q <= '9') {
          digit = *q - '0';
        } else if (*q >= 'a' && *q <= 'f') {
          digit = *q - 'a' + 10;
        } else if (*q >= 'A' && *q <= 'F') {
          digit = *q - 'A' + 10;
        } else {
          return 0;
        }
        TIXMLASSERT(digit < 16);
        TIXMLASSERT(digit == 0 || mult <= UINT_MAX / digit);
        const unsigned int digitScaled = mult * digit;
        TIXMLASSERT(ucs <= ULONG_MAX - digitScaled);
        ucs += digitScaled;
        TIXMLASSERT(mult <= UINT_MAX / 16);
        mult *= 16;
        --q;
      }
    } else {
      // Decimal.
      const char *q = p + 2;
      if (!(*q)) {
        return 0;
      }

      q = strchr(q, SEMICOLON);

      if (!q) {
        return 0;
      }
      TIXMLASSERT(*q == SEMICOLON);

      delta = q - p;
      --q;

      while (*q != '#') {
        if (*q >= '0' && *q <= '9') {
          const unsigned int digit = *q - '0';
          TIXMLASSERT(digit < 10);
          TIXMLASSERT(digit == 0 || mult <= UINT_MAX / digit);
          const unsigned int digitScaled = mult * digit;
          TIXMLASSERT(ucs <= ULONG_MAX - digitScaled);
          ucs += digitScaled;
        } else {
          return 0;
        }
        TIXMLASSERT(mult <= UINT_MAX / 10);
        mult *= 10;
        --q;
      }
    }
    // convert the UCS to UTF-8
    ConvertUTF32ToUTF8(ucs, value, length);
    return p + delta + 1;
  }
  return p + 1;
}

void XMLUtil::ToStr(int v, char *buffer, int bufferSize) {
  TIXML_SNPRINTF(buffer, bufferSize, "%d", v);
}

void XMLUtil::ToStr(unsigned v, char *buffer, int bufferSize) {
  TIXML_SNPRINTF(buffer, bufferSize, "%u", v);
}

void XMLUtil::ToStr(bool v, char *buffer, int bufferSize) {
  TIXML_SNPRINTF(buffer, bufferSize, "%s", v ? writeBoolTrue : writeBoolFalse);
}

/*
        ToStr() of a number is a very tricky topic.
        https://github.com/leethomason/tinyxml2/issues/106
*/
void XMLUtil::ToStr(float v, char *buffer, int bufferSize) {
  TIXML_SNPRINTF(buffer, bufferSize, "%.8g", v);
}

void XMLUtil::ToStr(double v, char *buffer, int bufferSize) {
  TIXML_SNPRINTF(buffer, bufferSize, "%.17g", v);
}

void XMLUtil::ToStr(int64_t v, char *buffer, int bufferSize) {
  // horrible syntax trick to make the compiler happy about %lld
  TIXML_SNPRINTF(buffer, bufferSize, "%lld", static_cast<long long>(v));
}

void XMLUtil::ToStr(uint64_t v, char *buffer, int bufferSize) {
  // horrible syntax trick to make the compiler happy about %llu
  TIXML_SNPRINTF(buffer, bufferSize, "%llu", (long long)v);
}

bool XMLUtil::ToInt(const char *str, int *value) {
  if (IsPrefixHex(str)) {
    unsigned v;
    if (TIXML_SSCANF(str, "%x", &v) == 1) {
      *value = static_cast<int>(v);
      return true;
    }
  } else {
    if (TIXML_SSCANF(str, "%d", value) == 1) {
      return true;
    }
  }
  return false;
}

bool XMLUtil::ToUnsigned(const char *str, unsigned *value) {
  if (TIXML_SSCANF(str, IsPrefixHex(str) ? "%x" : "%u", value) == 1) {
    return true;
  }
  return false;
}

bool XMLUtil::ToBool(const char *str, bool *value) {
  int ival = 0;
  if (ToInt(str, &ival)) {
    *value = (ival == 0) ? false : true;
    return true;
  }
  static const char *TRUE_VALS[] = {"true", "True", "TRUE", 0};
  static const char *FALSE_VALS[] = {"false", "False", "FALSE", 0};

  for (int i = 0; TRUE_VALS[i]; ++i) {
    if (StringEqual(str, TRUE_VALS[i])) {
      *value = true;
      return true;
    }
  }
  for (int i = 0; FALSE_VALS[i]; ++i) {
    if (StringEqual(str, FALSE_VALS[i])) {
      *value = false;
      return true;
    }
  }
  return false;
}

bool XMLUtil::ToFloat(const char *str, float *value) {
  if (TIXML_SSCANF(str, "%f", value) == 1) {
    return true;
  }
  return false;
}

bool XMLUtil::ToDouble(const char *str, double *value) {
  if (TIXML_SSCANF(str, "%lf", value) == 1) {
    return true;
  }
  return false;
}

bool XMLUtil::ToInt64(const char *str, int64_t *value) {
  if (IsPrefixHex(str)) {
    unsigned long long v =
        0; // horrible syntax trick to make the compiler happy about %llx
    if (TIXML_SSCANF(str, "%llx", &v) == 1) {
      *value = static_cast<int64_t>(v);
      return true;
    }
  } else {
    long long v =
        0; // horrible syntax trick to make the compiler happy about %lld
    if (TIXML_SSCANF(str, "%lld", &v) == 1) {
      *value = static_cast<int64_t>(v);
      return true;
    }
  }
  return false;
}

bool XMLUtil::ToUnsigned64(const char *str, uint64_t *value) {
  unsigned long long v =
      0; // horrible syntax trick to make the compiler happy about %llu
  if (TIXML_SSCANF(str, IsPrefixHex(str) ? "%llx" : "%llu", &v) == 1) {
    *value = (uint64_t)v;
    return true;
  }
  return false;
}

XMLPrinter::XMLPrinter(FILE *file, bool compact, int depth)
    : _elementJustOpened(false), _stack(), _firstElement(true), _fp(file),
      _depth(depth), _textDepth(-1), _processEntities(true),
      _compactMode(compact), _buffer() {
  for (int i = 0; i < ENTITY_RANGE; ++i) {
    _entityFlag[i] = false;
    _restrictedEntityFlag[i] = false;
  }
  for (int i = 0; i < NUM_ENTITIES; ++i) {
    const char entityValue = entities[i].value;
    const unsigned char flagIndex = static_cast<unsigned char>(entityValue);
    TIXMLASSERT(flagIndex < ENTITY_RANGE);
    _entityFlag[flagIndex] = true;
  }
  _restrictedEntityFlag[static_cast<unsigned char>('&')] = true;
  _restrictedEntityFlag[static_cast<unsigned char>('<')] = true;
  _restrictedEntityFlag[static_cast<unsigned char>('>')] =
      true; // not required, but consistency is nice
  _buffer.Push(0);
}

void XMLPrinter::Print(const char *format, ...) {
  va_list va;
  va_start(va, format);

  if (_fp) {
    vfprintf(_fp, format, va);
  } else {
    const int len = TIXML_VSCPRINTF(format, va);
    // Close out and re-start the va-args
    va_end(va);
    TIXMLASSERT(len >= 0);
    va_start(va, format);
    TIXMLASSERT(_buffer.Size() > 0 && _buffer[_buffer.Size() - 1] == 0);
    char *p = _buffer.PushArr(len) - 1; // back up over the null terminator.
    TIXML_VSNPRINTF(p, len + 1, format, va);
  }
  va_end(va);
}

void XMLPrinter::Write(const char *data, size_t size) {
  if (_fp) {
    fwrite(data, sizeof(char), size, _fp);
  } else {
    char *p = _buffer.PushArr(static_cast<int>(size)) -
              1; // back up over the null terminator.
    memcpy(p, data, size);
    p[size] = 0;
  }
}

void XMLPrinter::Putc(char ch) {
  if (_fp) {
    fputc(ch, _fp);
  } else {
    char *p =
        _buffer.PushArr(sizeof(char)) - 1; // back up over the null terminator.
    p[0] = ch;
    p[1] = 0;
  }
}

void XMLPrinter::PrintSpace(int depth) {
  for (int i = 0; i < depth; ++i) {
    Write("    ");
  }
}

void XMLPrinter::PrintString(const char *p, bool restricted) {
  // Look for runs of bytes between entities to print.
  const char *q = p;

  if (_processEntities) {
    const bool *flag = restricted ? _restrictedEntityFlag : _entityFlag;
    while (*q) {
      TIXMLASSERT(p <= q);
      // Remember, char is sometimes signed. (How many times has that bitten
      // me?)
      if (*q > 0 && *q < ENTITY_RANGE) {
        // Check for entities. If one is found, flush
        // the stream up until the entity, write the
        // entity, and keep looking.
        if (flag[static_cast<unsigned char>(*q)]) {
          while (p < q) {
            const size_t delta = q - p;
            const int toPrint =
                (INT_MAX < delta) ? INT_MAX : static_cast<int>(delta);
            Write(p, toPrint);
            p += toPrint;
          }
          bool entityFound = false;
          for (int i = 0; i < NUM_ENTITIES; ++i) {
            if (entities[i].value == *q) {
              Putc('&');
              Write(entities[i].pattern, entities[i].length);
              Putc(';');
              entityFound = true;
              break;
            }
          }
          if (!entityFound) {
            TIXMLASSERT(false);
          }
          ++p;
        }
      }
      ++q;
      TIXMLASSERT(q);
    }
  }
  if (p < q) {
    const size_t delta = q - p;
    const int toPrint = (INT_MAX < delta) ? INT_MAX : static_cast<int>(delta);
    Write(p, toPrint);
  }
}

void XMLPrinter::PushHeader(bool writeBOM, bool writeDec) {
  if (writeBOM) {
    static const unsigned char bom[] = {TIXML_UTF_LEAD_0, TIXML_UTF_LEAD_1,
                                        TIXML_UTF_LEAD_2, 0};
    Write(reinterpret_cast<const char *>(bom));
  }
  if (writeDec) {
    PushDeclaration("xml version=\"1.0\"");
  }
}

void XMLPrinter::PrepareForNewNode(bool compactMode) {
  SealElementIfJustOpened();

  if (compactMode) {
    return;
  }

  if (_firstElement) {
    PrintSpace(_depth);
  } else if (_textDepth < 0) {
    Putc('\n');
    PrintSpace(_depth);
  }

  _firstElement = false;
}

void XMLPrinter::OpenElement(const char *name, bool compactMode) {
  PrepareForNewNode(compactMode);
  _stack.Push(name);

  Write("<");
  Write(name);

  _elementJustOpened = true;
  ++_depth;
}

void XMLPrinter::PushAttribute(const char *name, const char *value) {
  TIXMLASSERT(_elementJustOpened);
  Putc(' ');
  Write(name);
  Write("=\"");
  PrintString(value, false);
  Putc('\"');
}

void XMLPrinter::PushAttribute(const char *name, int v) {
  char buf[BUF_SIZE];
  XMLUtil::ToStr(v, buf, BUF_SIZE);
  PushAttribute(name, buf);
}

void XMLPrinter::PushAttribute(const char *name, unsigned v) {
  char buf[BUF_SIZE];
  XMLUtil::ToStr(v, buf, BUF_SIZE);
  PushAttribute(name, buf);
}

void XMLPrinter::PushAttribute(const char *name, int64_t v) {
  char buf[BUF_SIZE];
  XMLUtil::ToStr(v, buf, BUF_SIZE);
  PushAttribute(name, buf);
}

void XMLPrinter::PushAttribute(const char *name, uint64_t v) {
  char buf[BUF_SIZE];
  XMLUtil::ToStr(v, buf, BUF_SIZE);
  PushAttribute(name, buf);
}

void XMLPrinter::PushAttribute(const char *name, bool v) {
  char buf[BUF_SIZE];
  XMLUtil::ToStr(v, buf, BUF_SIZE);
  PushAttribute(name, buf);
}

void XMLPrinter::PushAttribute(const char *name, double v) {
  char buf[BUF_SIZE];
  XMLUtil::ToStr(v, buf, BUF_SIZE);
  PushAttribute(name, buf);
}

void XMLPrinter::CloseElement(bool compactMode) {
  --_depth;
  const char *name = _stack.Pop();

  if (_elementJustOpened) {
    Write("/>");
  } else {
    if (_textDepth < 0 && !compactMode) {
      Putc('\n');
      PrintSpace(_depth);
    }
    Write("</");
    Write(name);
    Write(">");
  }

  if (_textDepth == _depth) {
    _textDepth = -1;
  }
  if (_depth == 0) {
    Putc('\n');
  }
  _elementJustOpened = false;
}

void XMLPrinter::SealElementIfJustOpened() {
  if (!_elementJustOpened) {
    return;
  }
  _elementJustOpened = false;
  Putc('>');
}

void XMLPrinter::PushText(const char *text, bool cdata) {
  _textDepth = _depth - 1;

  if (cdata) {
    SealElementIfJustOpened();
    Write("<![CDATA[");
    Write(text);
    Write("]]>");
  } else {
    SealElementIfJustOpened();
    PrintString(text, false);
  }
}

void XMLPrinter::PushText(int64_t value) {
  char buf[BUF_SIZE];
  XMLUtil::ToStr(value, buf, BUF_SIZE);
  PushText(buf, false);
}

void XMLPrinter::PushText(uint64_t value) {
  char buf[BUF_SIZE];
  XMLUtil::ToStr(value, buf, BUF_SIZE);
  PushText(buf, false);
}

void XMLPrinter::PushText(int value) {
  char buf[BUF_SIZE];
  XMLUtil::ToStr(value, buf, BUF_SIZE);
  PushText(buf, false);
}

void XMLPrinter::PushText(unsigned value) {
  char buf[BUF_SIZE];
  XMLUtil::ToStr(value, buf, BUF_SIZE);
  PushText(buf, false);
}

void XMLPrinter::PushText(bool value) {
  char buf[BUF_SIZE];
  XMLUtil::ToStr(value, buf, BUF_SIZE);
  PushText(buf, false);
}

void XMLPrinter::PushText(float value) {
  char buf[BUF_SIZE];
  XMLUtil::ToStr(value, buf, BUF_SIZE);
  PushText(buf, false);
}

void XMLPrinter::PushText(double value) {
  char buf[BUF_SIZE];
  XMLUtil::ToStr(value, buf, BUF_SIZE);
  PushText(buf, false);
}

void XMLPrinter::PushComment(const char *comment) {
  PrepareForNewNode(_compactMode);

  Write("<!--");
  Write(comment);
  Write("-->");
}

void XMLPrinter::PushDeclaration(const char *value) {
  PrepareForNewNode(_compactMode);

  Write("<?");
  Write(value);
  Write("?>");
}

void XMLPrinter::PushUnknown(const char *value) {
  PrepareForNewNode(_compactMode);

  Write("<!");
  Write(value);
  Putc('>');
}

bool XMLPrinter::VisitEnter(const XMLDocument &doc) {
  _processEntities = doc.ProcessEntities();
  if (doc.HasBOM()) {
    PushHeader(true, false);
  }
  return true;
}

bool XMLPrinter::VisitEnter(const XMLElement &element,
                            const XMLAttribute *attribute) {
  const XMLElement *parentElem = 0;
  if (element.Parent()) {
    parentElem = element.Parent()->ToElement();
  }
  const bool compactMode = parentElem ? CompactMode(*parentElem) : _compactMode;
  OpenElement(element.Name(), compactMode);
  while (attribute) {
    PushAttribute(attribute->Name(), attribute->Value());
    attribute = attribute->Next();
  }
  return true;
}

bool XMLPrinter::VisitExit(const XMLElement &element) {
  CloseElement(CompactMode(element));
  return true;
}

bool XMLPrinter::Visit(const XMLText &text) {
  PushText(text.Value(), text.CData());
  return true;
}

bool XMLPrinter::Visit(const XMLComment &comment) {
  PushComment(comment.Value());
  return true;
}

bool XMLPrinter::Visit(const XMLDeclaration &declaration) {
  PushDeclaration(declaration.Value());
  return true;
}

bool XMLPrinter::Visit(const XMLUnknown &unknown) {
  PushUnknown(unknown.Value());
  return true;
}

} // namespace tinyxml2
