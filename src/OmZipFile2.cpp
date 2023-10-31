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
#include "OmUtilStr.h"
#include "minizip-ng/mz.h"
#include "minizip-ng/mz_os.h"
#include "minizip-ng/mz_strm.h"
#include "minizip-ng/mz_zip.h"
#include "minizip-ng/mz_zip_rw.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmZipFile2.h"

/// \brief Zip status
///
/// Status definitions for Zip object
///
#define MZ_READER   0x1 //< Zip is in read mode
#define MZ_WRITER   0x2 //< Zip is in write mode
#define MZ_ERROR    0x4 //< Zip is in error state

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmZipFile2::OmZipFile2() :
  _mz(nullptr), _stat(0)
{

}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmZipFile2::~OmZipFile2()
{
  this->close();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmZipFile2::init(const wstring& path)
{
  this->close();

  this->_mz = mz_zip_writer_create();
  if(this->_mz == nullptr) {
    this->_stat |= MZ_ERROR;
    return false;
  }

  //mz_zip_writer_set_password(this->_mz, password);
  //mz_zip_writer_set_aes(this->_mz, FALSE);
  mz_zip_writer_set_compress_method(this->_mz, MZ_COMPRESS_METHOD_DEFLATE);
  mz_zip_writer_set_compress_level(this->_mz, MZ_COMPRESS_LEVEL_FAST);
  //mz_zip_writer_set_follow_links(this->_mz, options->follow_links);
  //mz_zip_writer_set_store_links(this->_mz, options->store_links);
  //mz_zip_writer_set_overwrite_cb(this->_mz, options, minizip_add_overwrite_cb);
  //mz_zip_writer_set_progress_cb(this->_mz, options, minizip_add_progress_cb);
  //mz_zip_writer_set_entry_cb(this->_mz, options, minizip_add_entry_cb);
  //mz_zip_writer_set_zip_cd(this->_mz, options->zip_cd);

  string utf8_path;
  Om_toUTF8(&utf8_path, path);

  if(mz_zip_writer_open_file(this->_mz, utf8_path.c_str(), 0, 0) != MZ_OK) {
    this->_stat |= MZ_ERROR;
    return false;
  }

  this->_stat |= MZ_WRITER;

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmZipFile2::load(const wstring& path)
{
  this->close();

  this->_mz = mz_zip_reader_create();
  if(this->_mz == nullptr) {
    this->_stat |= MZ_ERROR;
    return false;
  }

  string utf8_path;
  Om_toUTF8(&utf8_path, path);

  if(mz_zip_reader_open_file(this->_mz, utf8_path.c_str()) != MZ_OK) {
    this->_stat |= MZ_ERROR;
    return false;
  }

  this->_stat |= MZ_READER;

  return true;
}



///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmZipFile2::append(const wstring& src, const wstring& dst)
{
  if(_stat & MZ_WRITER) {

    string utf8_src, utf8_dst;

    Om_toUTF8(&utf8_src, src);
    Om_toUTF8(&utf8_dst, dst);

    if(mz_zip_writer_add_file(this->_mz, utf8_src.c_str(), utf8_dst.c_str()) != MZ_OK) {
      return false;
    }

    return true;
  }

  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmZipFile2::append(const void* data, size_t size, const wstring& dst)
{
  if(_stat & MZ_WRITER) {

    string utf8_dst;
    Om_toUTF8(&utf8_dst, dst);

    mz_zip_file file_info;

    memset(&file_info, 0, sizeof(mz_zip_file));

    file_info.filename = utf8_dst.c_str();
    /*
    file_info.modified_date
    file_info.accessed_date
    file_info.creation_date
    */
/*
    if(mz_zip_writer_add_buffer(this->_mz, data, size, utf8_dst, &file_info) != MZ_OK) {
      return false;
    }
*/
    return true;
  }

  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
unsigned OmZipFile2::indexCount() const
{
  int32_t mz_err;
  unsigned i = 0;

  if(this->_stat & MZ_READER) {

    mz_err = mz_zip_reader_goto_first_entry(this->_mz);
    if(mz_err != MZ_OK) {
      return i;
    }

    do {

      i++;

      mz_err = mz_zip_reader_goto_next_entry(this->_mz);
      if(mz_err != MZ_OK && mz_err != MZ_END_OF_LIST) {
        break;
      }

    } while(mz_err == MZ_OK);

  }

  return i;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
wstring OmZipFile2::index(unsigned i) const
{
  wstring ret;

  if(this->_stat & MZ_READER) {

    int32_t mz_err;

    mz_err = mz_zip_reader_goto_first_entry(this->_mz);
    if(mz_err != MZ_OK) {
      return ret;
    }

    for(unsigned k = 0; k < i; k++) {
      mz_err = mz_zip_reader_goto_next_entry(this->_mz);
      if(mz_err != MZ_OK) {
        break;
      }
    }

    mz_zip_file *file_info = nullptr;

    mz_zip_reader_entry_get_info(this->_mz, &file_info);

    Om_toUTF16(&ret, file_info->filename);

  }

  return ret;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmZipFile2::close()
{
  if(this->_stat & MZ_WRITER) {
    mz_zip_writer_close(this->_mz);
    mz_zip_writer_delete(&this->_mz);
  }

  if(this->_stat & MZ_READER) {
    mz_zip_reader_delete(&this->_mz);
  }

  this->_stat = 0;
  this->_mz = nullptr;
}
