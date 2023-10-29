/*
  This file is part of Open Mod Manager.

  Open Mod Manager is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Open Mod Manager is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Open Mod Manager. If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef OMUTILSTR_H_INCLUDED
#define OMUTILSTR_H_INCLUDED

#include "OmBase.h"


/// \brief Convert UTF-8 to UTF-16
///
/// Converts the given UTF-8 string to UTF-16.
///
/// \param[out] pwcs  : Pointer to wide string that receive converted data.
/// \param[in]  utf8  : UTF-8 text string to convert.
///
/// \return Count of written UTF-16 character.
///
size_t Om_toUTF16(wstring* pwcs, const string& utf8);

/// \brief Convert UTF-8 to UTF-16
///
/// Converts the given UTF-8 string to UTF-16.
///
/// \param[out] pwcs  : Pointer to wide string that receive converted data.
/// \param[in]  utf8  : UTF-8 text string to convert.
///
/// \return Count of written UTF-16 character.
///
size_t Om_toUTF16(wstring* pwcs, const char* utf8);

/// \brief Convert UTF-8 to UTF-16
///
/// Converts the given UTF-8 string to UTF-16.
///
/// \param[in]  utf8  : UTF-8 text string to convert.
///
/// \return Resulting UTF-16 wide string.
///
wstring Om_toUTF16(const string& utf8);

/// \brief Convert UTF-8 to UTF-16
///
/// Converts the given UTF-8 string to UTF-16.
///
/// \param[in]  utf8  : UTF-8 text string to convert.
///
/// \return Resulting UTF-16 wide string.
///
wstring Om_toUTF16(const char* utf8);

/// \brief Convert Unknown to UTF-16
///
/// Converts the given data to UTF-16, guessing the input encoding
/// among supported ones. Supported encoding are UTF-8, ISO-8859-1 and
/// Windows-1252.
///
/// \param[out] pwcs  : Pointer to wide string that receive converted data.
/// \param[in]  data  : Pointer to data to convert.
/// \param[in]  size  : Size of data.
///
/// \return Count of written UTF-16 character.
///
size_t Om_toUTF16(wstring* pwcs, const uint8_t* data, size_t size);

/// \brief Convert Unknown to UTF-16
///
/// Converts the given data to UTF-16, guessing the input encoding
/// among supported ones. Supported encoding are UTF-8, ISO-8859-1 and
/// Windows-1252.
///
/// \param[in]  data  : Pointer to data to convert.
/// \param[in]  size  : Size of data.
///
/// \return Resulting UTF-16 wide string.
///
wstring Om_toUTF16(const uint8_t* data, size_t size);

/// \brief Load text file and convert to UTF-16
///
/// Load the text file at specified path and converts it to UTF-16, guessing
/// the input encoding among supported ones. Supported encoding are UTF-8,
/// ISO-8859-1 and Windows-1252.
///
/// \param[out] pwcs  : Pointer to wide string that receive loaded text.
/// \param[in]  path  : Path of file to load.
///
/// \return Count of written UTF-16 codet.
///
size_t Om_loadToUTF16(wstring* pwcs, const wstring& path);

/// \brief UTF-16 to UTF-8 conversion.
///
/// Convert the given UTF-16 wide string into UTF-8 multibyte string.
///
/// \param[in]  utf8    : Pointer to char buffer that receives the converted data.
/// \param[in]  len     : Size of the supplied char buffer.
/// \param[in]  wstr    : Wide string to convert.
///
/// \return Count of written ANSI characters.
///
size_t Om_toUTF8(char* utf8, size_t len, const wstring& wstr);

/// \brief UTF-16 to UTF-8 conversion.
///
/// Convert the given UTF-16 wide string into UTF-8 multibyte string.
///
/// \param[in]  wstr    : Wide string to convert.
///
/// \return UTF-8 multibyte string conversion result.
///
string Om_toUTF8(const wstring& wstr);

/// \brief UTF-16 to UTF-8 conversion.
///
/// Convert the given UTF-16 wide char string into UTF-8 multibyte string.
///
/// \param[in]  utf8    : Pointer to multibyte string that receives the converted data.
/// \param[in]  wstr    : Wide char string to convert.
///
size_t Om_toUTF8(string* utf8, const wstring& wstr);

/// \brief Conversion to ANSI multibyte.
///
/// Convert the given wide char string into multibyte string using the
/// system default Windows ANSI code page.
///
/// \param[in]  ansi    : Pointer to buffer that receives the converted data.
/// \param[in]  len     : Size of the supplied buffer.
/// \param[in]  wstr    : Wide string to convert.
///
/// \return Count of written ANSI characters.
///
size_t Om_toANSI(char* ansi, size_t len, const wstring& wstr);

/// \brief Conversion to ANSI multibyte.
///
/// Convert the given wide char string into multibyte string using the
/// system default Windows ANSI code page.
///
/// \param[in]  ansi    : Pointer to multibyte string that receives the converted string.
/// \param[in]  wstr    : Wide string to convert.
///
/// \return Count of written ANSI characters.
///
size_t Om_toANSI(string* ansi, const wstring& wstr);

/// \brief Conversion from ANSI multibyte.
///
/// Convert the given multibyte string into wide char string using the
/// system default Windows ANSI code page.
///
/// \param[in]  wstr    : Pointer to wide string that receives the converted string.
/// \param[in]  ansi    : Pointer to buffer to convert.
///
/// \return Count of written wide characters.
///
size_t Om_fromAnsiCp(wstring* wstr, const char* ansi);

/// \brief Zip CDR entry to Windows path conversion.
///
/// Convert the given Unicode (UTF-8) Zip CDR entry into its corresponding
/// standard back-slash Windows path (UTF-16).
///
/// \param[in]  wstr    : Pointer to wide string that receives the converted string.
/// \param[in]  zcdr    : Pointer to null terminated UTF-8 char buffer to convert.
///
/// \return Count of read CDR entry characters.
///
size_t Om_fromZipCDR(wstring* wstr, const char* zcdr);

/// \brief Windows path to Zip CDR entry conversion.
///
/// Convert the given standard back-slash Windows path (UTF-16) into its
/// corresponding Unicode (UTF-8) Zip CDR entry.
///
/// \param[in]  zcdr    : Pointer to buffer to get conversion result.
/// \param[in]  len     : Size of the supplied char buffer.
/// \param[in]  wstr    : Standard Windows path to convert.
///
/// \return Count of written CDR entry characters.
///
size_t Om_toZipCDR(char* zcdr, size_t len, const wstring& wstr);

/// \brief Windows path to Zip CDR entry conversion.
///
/// Convert the given standard back-slash Windows path (UTF-16) into its
/// corresponding Unicode (UTF-8) Zip CDR entry.
///
/// \param[in]  zcdr    : Pointer to multibyte string that receives the converted data.
/// \param[in]  wstr    : Standard Windows path to convert.
///
/// \return Count of written CDR entry characters.
///
size_t Om_toZipCDR(string* zcdr, const wstring& wstr);

/// \brief Sort strings
///
/// Sorts the given array of strings in alphanumeric order.
///
/// \param[in]  string  : Pointer to vector array of wstring to sort.
///
//void Om_sortStrings(vector<wstring>* strings);

/// \brief Convert string to upper case.
///
/// Convert the given string to its upper case version.
///
/// \param[in]  str    : Pointer to string to convert.
///
void Om_strToUpper(wstring* str);

/// \brief Check if names matches
///
/// Checks whether two strings are equals in case insensitive way.
///
/// \param[in]  left    : First string to test.
/// \param[in]  right   : Second string to test.
///
/// \return True if strings are same despite unmatched case, false otherwise.
///
bool Om_namesMatches(const wstring& left, const wstring& right);

/// \brief Check if names matches
///
/// Checks whether two strings are equals in case insensitive way.
///
/// \param[in]  left    : First string to test.
/// \param[in]  right   : Second string to test.
///
/// \return True if strings are same despite unmatched case, false otherwise.
///
bool Om_namesMatches(const wstring& left, const wchar_t* right);


/// \brief Check URL validity
///
/// Checks whether the given string is a valid HTTP(S) URL.
///
/// \param[in]  url     : URL string to check.
///
/// \return True if the given string is a valid URL, false otherwise.
///
bool Om_isValidUrl(const wchar_t* url);

/// \brief Check URL validity
///
/// Checks whether the given string is a valid HTTP(S) URL.
///
/// \param[in]  url     : URL string to check.
///
/// \return True if the given string is a valid URL, false otherwise.
///
bool Om_isValidUrl(const wstring& url);

/// \brief Check file URL validity
///
/// Checks whether the given string is a valid HTTP(S) URL including
/// path to file.
///
/// \param[in]  url     : URL string to check.
///
/// \return True if the given string is a valid URL, false otherwise.
///
bool Om_isValidFileUrl(const wchar_t* url);

/// \brief Check file URL validity
///
/// Checks whether the given string is a valid HTTP(S) URL including
///  path to file.
///
/// \param[in]  url     : URL string to check.
///
/// \return True if the given string is a valid URL, false otherwise.
///
bool Om_isValidFileUrl(const wstring& url);

/// \brief Check URL path validity
///
/// Checks whether the given string is a valid HTTP(S) URL path.
///
/// \param[in]  path    : Path to check.
///
/// \return True if the given string is a valid URL path, false otherwise.
///
bool Om_isValidUrlPath(const wchar_t* path);

/// \brief Check URL path validity
///
/// Checks whether the given string is a valid HTTP(S) URL path.
///
/// \param[in]  path    : Path to check.
///
/// \return True if the given string is a valid URL path, false otherwise.
///
bool Om_isValidUrlPath(const wstring& path);

/// \brief Check file name validity
///
/// Checks whether the given string is suitable for file name or
/// contains illegal character(s).
///
/// \param[in]  name    : File name to check.
///
/// \return True if the given string is suitable for file name, false otherwise.
///
bool Om_isValidName(const wchar_t* name);

/// \brief Check file name validity
///
/// Checks whether the given string is suitable for Windows file name or
/// contains illegal character(s).
///
/// \param[in]  name    : File name to check.
///
/// \return True if the given string is suitable for file name, false otherwise.
///
bool Om_isValidName(const wstring& name);

/// \brief Check file path validity
///
/// Checks whether the given string is suitable for file path or
/// contains illegal character(s).
///
/// \param[in]  path    : File path to check.
///
/// \return True if the given string is suitable for file path, false otherwise.
///
bool Om_isValidPath(const wchar_t* path);

/// \brief Check file path validity
///
/// Checks whether the given string is suitable for Windows file path or
/// contains illegal character(s).
///
/// \param[in]  path    : File path to check.
///
/// \return True if the given string is suitable for file path, false otherwise.
///
bool Om_isValidPath(const wstring& path);

/// \brief Extract file extension from path
///
/// Extract and returns the file extension part of the given full file path
/// or file name.
///
/// \param[in]  path    : Windows formated file path or name.
///
/// \return Extracted extension without the separating dot.
///
wstring Om_getFileExtPart(const wstring& path);

/// \brief Extract file part from path
///
/// Extracts and returns the path part that follows the last
/// separator (backslash). Assuming the given path is a full file
/// path, the file name with its extension will be extracted.
///
/// \param[in]  path    : Windows formated full path.
///
/// \return Extracted path part.
///
wstring Om_getFilePart(const wstring& path);

/// \brief Extract directory part from path
///
/// Extracts and returns the path part that precede the last
/// separator (backslash), including the last separator. Assuming the given
/// path is a full file path, the full path without the file name will be
/// extracted.
///
/// \param[in]  path    : Windows formated file path.
///
/// \return Extracted path part.
///
wstring Om_getDirPart(const wstring& uri);


/// \brief Extract file name without extension from path
///
/// Extracts and returns the file name, without its extension, from
/// the given full path.
///
/// \param[in]  path    : Windows formated file path.
///
/// \return Extracted file name without extension.
///
wstring Om_getNamePart(const wstring& uri);

/// \brief Check if for file name extension matches
///
/// Checks whether the given file has the specified extension.
///
/// \param[in]  file    : file name or full path to test extension.
/// \param[in]  ext     : Three or four letters extension to check for.
///
/// \return True if given filename has the requested extension, false otherwise.
///
bool Om_extensionMatches(const wstring& file, const wchar_t* ext);

/// \brief Concatenate paths
///
/// Concatenates two paths, adding separator if necessary.
///
/// \param[in]  left    : Left path part to concatenate.
/// \param[in]  right   : Right path part to concatenate.
///
/// \return Result of the concatenation.
///
wstring Om_concatPaths(const wstring& left, const wstring& right);

/// \brief Concatenate paths
///
/// Concatenates two paths, adding separator if necessary.
///
/// \param[out] conc    : String to be set as the result of concatenation.
/// \param[in]  left    : Left path part to concatenate.
/// \param[in]  right   : Right path part to concatenate.
///
void Om_concatPaths(wstring& conc, const wstring& left, const wstring& right);

/// \brief Compare path roots
///
/// Checks whether the given path has the specified root.
///
/// \param[in]  root    : Root path for verification.
/// \param[in]  item    : Path to verify if it has the specified root.
///
/// \return True if path actually has the specified root, false otherwise.
///
bool Om_isRootOfPath(const wstring& root, const wstring& path);

/// \brief Get relative part of path
///
/// Checks whether the given path has the specified root, then extract its
/// relative part.
///
/// \param[out] rel     : String to be set as the extracted relative path.
/// \param[in]  root    : Root of path used to check and extract relative part.
/// \param[in]  item    : Path to be checked and to get the relative part from.
///
/// \return True if path actually has the specified root and operation succeed,
///         false otherwise.
///
bool Om_getRelativePath(wstring& rel, const wstring& root, const wstring& path);

/// \brief Concatenate paths
///
/// Concatenates two paths, adding separator if necessary.
///
/// \param[in]  left    : Left path part to concatenate.
/// \param[in]  right   : Right path part to concatenate.
///
/// \return Result of the concatenation.
///
wstring Om_concatURLs(const wstring& left, const wstring& right);

/// \brief Concatenate paths
///
/// Concatenates two paths, adding separator if necessary.
///
/// \param[out] conc    : String to be set as the result of concatenation.
/// \param[in]  left    : Left path part to concatenate.
/// \param[in]  right   : Right path part to concatenate.
///
void Om_concatURLs(wstring& conc, const wstring& left, const wstring& right);

/// \brief Get formated bytes/octets size string
///
/// Create a formated string of the given size in bytes in human readable
/// form with proper unit conversion.
///
/// \param[in]  bytes   : Size in bytes.
/// \param[in]  octet   : Write string using O for octet instead of B for bytes.
///
/// \return Formated string describing size.
///
wstring Om_formatSizeStr(size_t bytes, bool octet = false);

/// \brief Get system formated bytes size string
///
/// Create a formated string of the given size in bytes as system wide standard.
///
/// \param[in]  bytes   : Size in bytes.
/// \param[in]  kbytes  : Use kilobytes representation.
///
/// \return Formated string describing size.
///
wstring Om_formatSizeSysStr(uint64_t bytes, bool kbytes = true);

/// \brief Check whether is version string
///
/// Checks whether the given string can be parsed as valid version number(s).
///
/// \param[in] str       : String to test.
///
/// \return True if the given string can be parsed as version number(s).
///
bool Om_strIsVersion(const wstring& str);

/// \brief Escape string for Markdown
///
/// Escapes Markdown specific character the given source string then
/// append it to the specified destination.
///
/// \param[in] dst       : Destination string to append escaped text.
/// \param[in] str       : Source string to escape.
///
void Om_escapeMarkdown(wstring* dst, const wstring& src);

/// \brief Escape string for Markdown
///
/// Escapes Markdown specific character the given source string to
/// the specified destination.
///
/// \param[in] buf       : Destination buffer that receive escaped text.
/// \param[in] str       : Source string to escape.
///
/// \return Length of escaped string
///
size_t Om_escapeMarkdown(wchar_t* buf, const wstring& src);

#endif // OMUTILSTR_H_INCLUDED
