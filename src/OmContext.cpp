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
#include "OmBase.h"
#include <algorithm>            //< std::sort

#include "OmBaseApp.h"

#include "OmManager.h"
#include "OmModChan.h"

#include "OmUtilFs.h"
#include "OmUtilHsh.h"
#include "OmUtilErr.h"
#include "OmUtilStr.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmContext.h"


/// \brief Mod Channel index comparison callback
///
/// std::sort callback comparison function for sorting Mod Channels
/// by index number order.
///
/// \param[in]  a     : Left Mod Channel.
/// \param[in]  b     : Right Mod Channel.
///
/// \return True if Mod Channel a is "before" Mod Channel b, false otherwise
///
static bool __chn_sort_index_fn(const OmModChan* a, const OmModChan* b)
{
  return (a->index() < b->index());
}


/// \brief OmBatch index comparison callback
///
/// std::sort callback comparison function for sorting Mod Batches
/// by index number order.
///
/// \param[in]  a     : Left OmBatch.
/// \param[in]  b     : Right OmBatch.
///
/// \return True if OmBatch a is "before" OmBatch b, false otherwise
///
static bool __bat_sort_index_fn(const OmBatch* a, const OmBatch* b)
{
  return (a->index() < b->index());
}




///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmContext::OmContext(OmManager* pMgr) :
  _manager(pMgr), _config(), _path(), _uuid(), _title(), _home(), _icon(nullptr),
  _chnLs(), _chnCur(-1), _batLs(), _batQuietMode(true), _valid(false), _error()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmContext::~OmContext()
{
  this->close();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmContext::open(const wstring& path)
{
  wstring verbose; //< for log message compositing

  this->close();

  // try to open and parse the XML file
  if(!this->_config.open(path, OMM_XMAGIC_CTX)) {
    this->_error = Om_errParse(L"Definition file", path, this->_config.lastErrorStr());
    this->log(0, L"Context(<anonymous>) Open", this->_error);
    return false;
  }

  // check for the presence of <uuid> entry
  if(!this->_config.xml().hasChild(L"uuid")) {
    this->_error =  L"\""+Om_getFilePart(path)+L"\" invalid definition: <uuid> node missing.";
    log(0, L"Context(<anonymous>) Open", this->_error);
    return false;
  }

  // check for the presence of <title> entry
  if(!this->_config.xml().hasChild(L"title")) {
    this->_error = L"\""+Om_getFilePart(path)+L"\" invalid definition: <title> node missing.";
    log(0, L"Context(<anonymous>) Open", this->_error);
    return false;
  }

  // right now this Context appear usable, even if it is empty
  this->_path = path;
  this->_home = Om_getDirPart(this->_path);
  this->_uuid = this->_config.xml().child(L"uuid").content();
  this->_title = this->_config.xml().child(L"title").content();
  this->_valid = true;

  this->log(2, L"Context("+this->_title+L") Open",
            L"Definition parsed.");

  // lookup for a icon
  if(this->_config.xml().hasChild(L"icon")) {

    // we got a banner
    wstring ico_path = this->_config.xml().child(L"icon").content();

    this->log(2, L"Context("+this->_title+L") Open",
              L"Associated icon \""+ico_path+L"\"");

    HICON hIc = nullptr;
    ExtractIconExW(ico_path.c_str(), 0, &hIc, nullptr, 1); //< large icon
    //ExtractIconExW(ico_path.c_str(), 0, nullptr, &hIc, 1); //< small icon

    if(hIc) {
      this->_icon = hIc;
    } else {
      this->log(1, L"Context("+this->_title+L") Open",
                L"\""+ico_path+L"\" icon extraction failed.");
    }
  }

  // we check for saved batches quiet mode option
  if(this->_config.xml().hasChild(L"batches_quietmode")) {
    this->_batQuietMode = this->_config.xml().child(L"batches_quietmode").attrAsInt(L"enable");
  } else {
    this->setBatQuietMode(this->_batQuietMode); //< create default
  }

  // load Mod Channels for this Context
  vector<wstring> file_ls;
  vector<wstring> subdir_ls;
  Om_lsDir(&subdir_ls, this->_home, false);

  if(subdir_ls.size()) {

    OmConfig cfg;

    for(size_t i = 0; i < subdir_ls.size(); ++i) {

      // check for presence of Mod Channel definition file
      file_ls.clear();
      Om_lsFileFiltered(&file_ls, this->_home+L"\\"+subdir_ls[i], L"*." OMM_LOC_DEF_FILE_EXT, true);

      // we parse the fist definition file found in directory
      if(file_ls.size()) {

        this->log(2, L"Context("+this->_title+L") Open",
                  L"Linking Mod Channel \""+Om_getFilePart(file_ls[0])+L"\"");

        // we use the first file we found
        OmModChan* pChn = new OmModChan(this);

        if(pChn->open(file_ls[0])) {
          this->_chnLs.push_back(pChn);
        } else {
          delete pChn;
        }
      }
    }
  }

  // Search for Installation Batches within Context home and subfolders
  file_ls.clear();
  Om_lsFileFiltered(&file_ls, this->_home, L"*." OMM_BAT_DEF_FILE_EXT, true);
  for(size_t i = 0; i < subdir_ls.size(); ++i) {
    Om_lsFileFiltered(&file_ls, this->_home+L"\\"+subdir_ls[i], L"*." OMM_BAT_DEF_FILE_EXT, true);
  }

  if(file_ls.size()) {

    for(size_t i = 0; i < file_ls.size(); ++i) {

      this->log(2, L"Context("+this->_title+L") Open",
                L"Bind Batch \""+Om_getFilePart(file_ls[i])+L"\"");

      OmBatch* pBat = new OmBatch(this);

      if(pBat->open(file_ls[i])) {
        this->_batLs.push_back(pBat);
      } else {
        delete pBat;
      }
    }
  }

  // sort Mod Channel by index
  if(this->_chnLs.size() > 1)
    sort(this->_chnLs.begin(), this->_chnLs.end(), __chn_sort_index_fn);

  // sort Batches by index
  if(this->_batLs.size() > 1)
    sort(this->_batLs.begin(), this->_batLs.end(), __bat_sort_index_fn);

  // the first location in list become the default active one
  if(this->_chnLs.size()) {
    this->chnSel(0);
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmContext::close()
{
  if(this->_valid) {

    wstring title = this->_title;

    this->_path.clear();
    this->_home.clear();
    this->_title.clear();

    if(this->_icon) DestroyIcon(this->_icon);
    this->_icon = nullptr;

    this->_config.close();

    for(size_t i = 0; i < this->_chnLs.size(); ++i)
      delete this->_chnLs[i];

    this->_chnLs.clear();

    this->_chnCur = -1;

    for(size_t i = 0; i < this->_batLs.size(); ++i)
      delete this->_batLs[i];

    this->_batLs.clear();

    this->_valid = false;

    this->log(2, L"Context("+title+L") Close",
              L"Success");
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmContext::setTitle(const wstring& title)
{
  if(this->_config.valid()) {

    this->_title = title;

    if(this->_config.xml().hasChild(L"title")) {
      this->_config.xml().child(L"title").setContent(title);
    } else {
      this->_config.xml().addChild(L"title").setContent(title);
    }

    this->_config.save();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmContext::setIcon(const wstring& src)
{
  if(this->_config.valid()) {

    // delete previous object
    if(this->_icon) DestroyIcon(this->_icon);
    this->_icon = nullptr;

    // empty source path mean remove icon
    if(!src.empty()) {

      HICON hIc = nullptr;

      if(Om_isFile(src))
        ExtractIconExW(src.c_str(), 0, &hIc, nullptr, 1);

      if(hIc) {

        this->_icon = hIc;

        if(this->_config.xml().hasChild(L"icon")) {
          this->_config.xml().child(L"icon").setContent(src);
        } else {
          this->_config.xml().addChild(L"icon").setContent(src);
        }

      } else {
        this->log(1, L"Context("+this->_title+L") Set Icon",
                  L"\""+src+L"\" icon extraction failed.");
      }
    }

    if(!this->_icon) {

      if(this->_config.xml().hasChild(L"icon")) {
        this->_config.xml().remChild(this->_config.xml().child(L"icon"));
      }
    }

    this->_config.save();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmModChan* OmContext::chnGet(const wstring& uuid)
{
  for(size_t i = 0; i < this->_chnLs.size(); ++i) {
    if(uuid == this->_chnLs[i]->uuid())
      return this->_chnLs[i];
  }

  return nullptr;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmContext::chnSort()
{
  if(this->_chnLs.size() > 1)
    sort(this->_chnLs.begin(), this->_chnLs.end(), __chn_sort_index_fn);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmContext::chnSel(int i)
{
  if(i >= 0) {
    if(i < (int)this->_chnLs.size()) {
      this->_chnCur = i;
      this->log(2, L"Context("+this->_title+L") Select Mod Channel",
                L"\""+this->_chnLs[_chnCur]->title()+L"\".");
    } else {
      return false;
    }
  } else {
    this->_chnCur = -1;
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmContext::chnSel(const wstring& uuid)
{
  for(size_t i = 0; i < this->_chnLs.size(); ++i) {
    if(uuid == this->_chnLs[i]->uuid()) {
      this->_chnCur = i;
      this->log(2, L"Context("+this->_title+L") Select Mod Channel",
                L"\""+this->_chnLs[_chnCur]->title()+L"\".");
      return true;
    }
  }

  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
int OmContext::chnIndex(const wstring& uuid)
{
  for(size_t i = 0; i < this->_chnLs.size(); ++i) {
    if(uuid == this->_chnLs[i]->uuid()) {
      return i;
    }
  }

  return -1;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmContext::chnAdd(const wstring& title, const wstring& install, const wstring& library, const wstring& backup)
{
  // this theoretically can't happen, but we check to be sure
  if(!this->isValid()) {
    this->_error = L"Context is empty.";
    this->log(0, L"Context(<anonymous>) Create Mod Channel", this->_error);
    return false;
  }

  int result;

  // compose Mod Channel home path
  wstring chn_home = this->_home + L"\\" + title;

  // create Mod Channel sub-folder
  if(!Om_isDir(chn_home)) {
    result = Om_dirCreate(chn_home);
    if(result != 0) {
      this->_error = Om_errCreate(L"Mod Channel home", chn_home, result);
      this->log(0, L"Context("+this->_title+L") Create Mod Channel", this->_error);
      return false;
    }
  } else {
    this->log(1, L"Context("+this->_title+L") Create Mod Channel",
              Om_errExists(L"Mod Channel home",chn_home));
  }

  // compose Mod Channel definition file name
  wstring chn_def_path = chn_home + L"\\" + title;
  chn_def_path += L"."; chn_def_path += OMM_LOC_DEF_FILE_EXT;

  // check whether definition file already exists and delete it
  if(Om_isFile(chn_def_path)) {

    this->log(1, L"Context("+this->_title+L") Create Mod Channel",
              Om_errExists(L"Definition file",chn_def_path));

    int result = Om_fileDelete(chn_def_path);
    if(result != 0) {
      this->_error = Om_errDelete(L"Old definition file", chn_def_path, result);
      this->log(0, L"Context("+this->_title+L") Create Mod Channel", this->_error);
      return false;
    }
  }

  // initialize new definition file
  OmConfig chn_def;
  if(!chn_def.init(chn_def_path, OMM_XMAGIC_CHN)) {
    this->_error = Om_errInit(L"Definition file", chn_def_path, chn_def.lastErrorStr());
    this->log(0, L"Context("+this->_title+L") Create Mod Channel", this->_error);
    return false;
  }

  // Generate a new UUID for this Mod Channel
  wstring uuid = Om_genUUID();

  // Get XML document instance
  OmXmlNode chn_xml = chn_def.xml();

  // define uuid and title in definition file
  chn_xml.addChild(L"uuid").setContent(uuid);
  chn_xml.addChild(L"title").setContent(title);

  // define ordering index in definition file
  chn_xml.child(L"title").setAttr(L"index", static_cast<int>(this->_chnLs.size()));

  // define installation destination folder in definition file
  chn_xml.addChild(L"install").setContent(install);

  // checks whether we have custom Backup folder
  if(backup.empty()) {
    // Create the default backup sub-folder
    Om_dirCreate(chn_home + L"\\Backup");
  } else {
    // check whether custom Library folder exists
    if(!Om_isDir(backup)) {
      this->log(1, L"Context("+this->_title+L") Create Mod Channel",
                Om_errIsDir(L"Custom Backup folder", backup));
    }
    // add custom backup in definition
    chn_xml.addChild(L"backup").setContent(backup);
  }

  // checks whether we have custom Library folder
  if(library.empty()) {
    // Create the default library sub-folder
    Om_dirCreate(chn_home + L"\\Library");
  } else {
    // check whether custom Library folder exists
    if(!Om_isDir(library)) {
      this->log(1, L"Context("+this->_title+L") Create Mod Channel",
                Om_errIsDir(L"Custom Library folder", library));
    }
    // add custom library in definition
    chn_xml.addChild(L"library").setContent(library);
  }

  // save and close definition file
  chn_def.save();
  chn_def.close();

  this->log(2, L"Context("+this->_title+L") Create Mod Channel", L"Mod Channel \""+title+L")\" created.");

  // load the newly created Mod Channel
  OmModChan* pChn = new OmModChan(this);
  pChn->open(chn_def_path);
  this->_chnLs.push_back(pChn);

  // sort locations by index
  this->chnSort();

  // select the last added location
  this->chnSel(this->_chnLs.size() - 1);

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmContext::chnRem(unsigned id)
{
  if(id >= this->_chnLs.size())
    return false;

  OmModChan* pChn = this->_chnLs[id];

  if(pChn->bckHasData()) {
    this->_error = L"Aborted: Still have backup data to be restored.";
    this->log(0, L"Context("+this->_title+L") Delete Mod Channel", this->_error);
    return false;
  }

  bool has_error = false;

  // keep Mod Channel paths
  wstring chn_name = pChn->title();
  wstring chn_home = pChn->home();
  wstring chn_path = pChn->path();

  // close Mod Channel
  pChn->close();

  // remove the default backup folder
  wstring bck_path = chn_home + L"\\Backup";
  if(Om_isDir(bck_path)) {
    // this will fails if folder not empty, this is intended
    int result = Om_dirDelete(bck_path);
    if(result != 0) {
      this->log(1, L"Context("+this->_title+L") Delete Mod Channel",
                Om_errDelete(L"Backup folder", bck_path, result));
    }
  }

  // remove the default Library folder
  wstring lib_path = chn_home + L"\\Library";
  if(Om_isDir(lib_path)) {
    // this will fails if folder not empty, this is intended
    if(Om_isDirEmpty(lib_path)) {
      int result = Om_dirDelete(lib_path);
      if(result != 0) {
        this->log(1, L"Context("+this->_title+L") Delete Mod Channel",
                  Om_errDelete(L"Library folder", lib_path, result));
      }
    } else {
      this->log(1, L"Context("+this->_title+L") Delete Mod Channel",
              L"Non-empty Library folder will not be deleted");
    }
  }

  // remove the definition file
  if(Om_isFile(chn_path)) {
    // close the definition file
    this->_config.close();
    int result = Om_fileDelete(chn_path);
    if(result != 0) {
      this->_error = Om_errDelete(L"Definition file", chn_path, result);
      this->log(1, L"Context("+this->_title+L") Delete Mod Channel", this->_error);
      has_error = true; //< this is considered as a real error
    }
  }

  // check if location home folder is empty, if yes, we delete it
  if(Om_isDirEmpty(chn_home)) {
    int result = Om_dirDelete(chn_home);
    if(result != 0) {
      this->_error = Om_errDelete(L"Home folder", chn_home, result);
      this->log(1, L"Context("+this->_title+L") Delete Mod Channel", this->_error);
      has_error = true; //< this is considered as a real error
    }
  } else {
    this->log(1, L"Context("+this->_title+L") Delete Mod Channel",
              L"Non-empty home folder \""+chn_home+L"\" will not be deleted");
  }

  this->log(2, L"Context("+this->_title+L") Delete Mod Channel",
            L"Mod Channel \""+chn_name+L"\" deleted.");

  // delete object
  delete pChn;

  // remove from list
  this->_chnLs.erase(this->_chnLs.begin()+id);

  // update locations order indexing
  for(size_t i = 0; i < this->_chnLs.size(); ++i) {
    this->_chnLs[i]->setIndex(i);
  }

  // sort Mod Channels by index
  this->chnSort();

  // select the first available location
  this->chnSel(0);

  return !has_error;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmContext::batSort()
{
  if(this->_batLs.size() > 1)
    sort(this->_batLs.begin(), this->_batLs.end(), __bat_sort_index_fn);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmBatch* OmContext::batAdd(const wstring& title)
{
  // compose path using title and context home
  wstring path = this->_home + L"\\";
  path += title; path += L"."; path += OMM_BAT_DEF_FILE_EXT;

  // Create new batch object
  OmBatch* pBat = new OmBatch(this);
  pBat->init(path, title, this->_batLs.size());
  this->_batLs.push_back(pBat);

  this->log(2, L"Context("+this->_title+L") Create Batch", L"Batch \""+title+L"\" created.");

  // sort Batches by index
  this->batSort();

  return pBat;
}



///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmContext::batRem(unsigned id)
{
  if(id < this->_batLs.size()) {

    OmBatch* pBat = this->_batLs[id];

    wstring bat_path = pBat->path();
    wstring bat_title = pBat->title();


    // remove the definition file
    if(Om_isFile(bat_path)) {
      // close the definition file
      this->_config.close();
      int result = Om_fileDelete(bat_path);
      if(result != 0) {
        this->_error = Om_errDelete(L"Batch definition file", bat_path, result);
        this->log(1, L"Context("+this->_title+L") Delete Batch", this->_error);
        return false;
      }
    }

    // delete batch object
    delete pBat;

    // remove from list
    this->_batLs.erase(this->_batLs.begin()+id);

    // update batches order indexing
    for(size_t i = 0; i < this->_batLs.size(); ++i) {
      this->_batLs[i]->setIndex(i);
    }

    // sort Batches by index
    this->batSort();

    this->log(2, L"Context("+this->_title+L") Delete Batch", L"Installation Batch \""+bat_title+L"\" deleted.");

    return true;
  }

  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmContext::setBatQuietMode(bool enable)
{
  this->_batQuietMode = enable;

  if(this->_config.valid()) {

    if(this->_config.xml().hasChild(L"batches_quietmode")) {
      this->_config.xml().child(L"batches_quietmode").setAttr(L"enable", this->_batQuietMode ? 1 : 0);
    } else {
      this->_config.xml().addChild(L"batches_quietmode").setAttr(L"enable", this->_batQuietMode ? 1 : 0);
    }

    this->_config.save();
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmContext::log(unsigned level, const wstring& head, const wstring& detail)
{
  wstring log_str = L"Manager:: "; log_str.append(head);

  this->_manager->log(level, log_str, detail);
}
