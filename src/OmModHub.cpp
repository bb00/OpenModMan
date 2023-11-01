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
#include "OmModHub.h"


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
OmModHub::OmModHub(OmManager* pMgr) :
  _manager(pMgr), _config(), _path(), _uuid(), _title(), _home(), _icon(nullptr),
  _modChanLs(), _modChanSl(-1), _batLs(), _batQuietMode(true), _valid(false), _error()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmModHub::~OmModHub()
{
  this->close();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModHub::open(const wstring& path)
{
  vector<wstring> file_ls;
  vector<wstring> fold_ls;

  wstring temp_path, verbose; //< for log message compositing

  this->close();

  // Migrate old standard to new standard
  if(!this->_migrate(path)) {
    this->_error = L"Migration is required but has failed, see log for more details";
    this->log(0, L"ModHub(<anonymous>) Open", this->_error);
    return false;
  }

  // Check whether path is valid
  if(!Om_isDir(path)) {
    this->_error = L"The specified folder doesn't exists";
    this->log(0, L"ModHub(<anonymous>) Open", this->_error);
    return false;
  }

  // Expected Mod Hub definition file path
  temp_path = path + L"\\ModHub.xml";

  // Check whether definition file exists
  if(!Om_isFile(temp_path)) {
    this->_error = L"Definition file (\"ModHub.xml\") not found";
    this->log(0, L"ModHub(<anonymous>) Open", this->_error);
    return false;
  }

  // try to open and parse the XML file
  if(!this->_config.open(temp_path, OMM_XMAGIC_HUB)) {
    this->_error = Om_errParse(L"Definition file", path, this->_config.lastErrorStr());
    this->log(0, L"ModHub(<anonymous>) Open", this->_error);
    return false;
  }

  // check for the presence of <uuid> entry
  if(!this->_config.xml().hasChild(L"uuid")) {
    this->_error =  L"\""+Om_getFilePart(path)+L"\" invalid definition: <uuid> node missing.";
    log(0, L"ModHub(<anonymous>) Open", this->_error);
    return false;
  }

  // check for the presence of <title> entry
  if(!this->_config.xml().hasChild(L"title")) {
    this->_error = L"\""+Om_getFilePart(path)+L"\" invalid definition: <title> node missing.";
    log(0, L"ModHub(<anonymous>) Open", this->_error);
    return false;
  }

  // right now this Mod Hub appear usable, even if it is empty
  this->_home = path;
  this->_path = temp_path;
  this->_uuid = this->_config.xml().child(L"uuid").content();
  this->_title = this->_config.xml().child(L"title").content();
  this->_valid = true;

  this->log(2, L"ModHub("+this->_title+L") Open",
            L"Definition parsed.");

  // lookup for a icon
  if(this->_config.xml().hasChild(L"icon")) {

    // we got a banner
    wstring ico_path = this->_config.xml().child(L"icon").content();

    this->log(2, L"ModHub("+this->_title+L") Open",
              L"Associated icon \""+ico_path+L"\"");

    HICON hIc = nullptr;
    ExtractIconExW(ico_path.c_str(), 0, &hIc, nullptr, 1); //< large icon
    //ExtractIconExW(ico_path.c_str(), 0, nullptr, &hIc, 1); //< small icon

    if(hIc) {
      this->_icon = hIc;
    } else {
      this->log(1, L"ModHub("+this->_title+L") Open",
                L"\""+ico_path+L"\" icon extraction failed.");
    }
  }

  // we check for saved batches quiet mode option
  if(this->_config.xml().hasChild(L"batches_quietmode")) {
    this->_batQuietMode = this->_config.xml().child(L"batches_quietmode").attrAsInt(L"enable");
  } else {
    this->setBatQuietMode(this->_batQuietMode); //< create default
  }

  // load Mod Channels for this Mod Hub
  fold_ls.clear();
  Om_lsDir(&fold_ls, this->_home, true);

  for(size_t i = 0; i < fold_ls.size(); ++i) {

    // Expected Mod Chan definition file path
    temp_path = fold_ls[i] + L"\\ModChan.xml";

    if(Om_isFile(temp_path)) {

      this->log(2, L"ModHub("+this->_title+L") Open",
                L"Linking Mod Channel \""+Om_getFilePart(fold_ls[i])+L"\"");

      // we use the first file we found
      OmModChan* pModChan = new OmModChan(this);

      if(pModChan->open(temp_path)) {
        this->_modChanLs.push_back(pModChan);
      } else {
        delete pModChan;
      }
    }
  }

  // load Script for this Mod Hub

  // Expected Script library path
  temp_path = this->_home + L"\\_Scripts";

  file_ls.clear();
  Om_lsFileFiltered(&file_ls, temp_path, L"*.xml", true);

  for(size_t i = 0; i < file_ls.size(); ++i) {

    this->log(2, L"ModHub("+this->_title+L") Open",
              L"Bind Script \""+Om_getFilePart(file_ls[i])+L"\"");

    OmBatch* pBat = new OmBatch(this);

    if(pBat->open(file_ls[i])) {
      this->_batLs.push_back(pBat);
    } else {
      delete pBat;
    }
  }

  // sort Mod Channel by index
  if(this->_modChanLs.size() > 1)
    sort(this->_modChanLs.begin(), this->_modChanLs.end(), __chn_sort_index_fn);

  // sort Batches by index
  if(this->_batLs.size() > 1)
    sort(this->_batLs.begin(), this->_batLs.end(), __bat_sort_index_fn);

  // the first location in list become the default active one
  if(this->_modChanLs.size()) {
    this->modChanSelect(0);
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModHub::close()
{
  if(this->_valid) {

    wstring title = this->_title;

    this->_path.clear();
    this->_home.clear();
    this->_title.clear();

    if(this->_icon) DestroyIcon(this->_icon);
    this->_icon = nullptr;

    this->_config.close();

    for(size_t i = 0; i < this->_modChanLs.size(); ++i)
      delete this->_modChanLs[i];

    this->_modChanLs.clear();

    this->_modChanSl = -1;

    for(size_t i = 0; i < this->_batLs.size(); ++i)
      delete this->_batLs[i];

    this->_batLs.clear();

    this->_valid = false;

    this->log(2, L"ModHub("+title+L") Close",
              L"Success");
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModHub::setTitle(const wstring& title)
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
void OmModHub::setIcon(const wstring& src)
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
        this->log(1, L"ModHub("+this->_title+L") Set Icon",
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
OmModChan* OmModHub::modChanGet(const wstring& uuid)
{
  for(size_t i = 0; i < this->_modChanLs.size(); ++i) {
    if(uuid == this->_modChanLs[i]->uuid())
      return this->_modChanLs[i];
  }

  return nullptr;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModHub::modChanSort()
{
  if(this->_modChanLs.size() > 1)
    sort(this->_modChanLs.begin(), this->_modChanLs.end(), __chn_sort_index_fn);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModHub::modChanSelect(int i)
{
  if(i >= 0) {
    if(i < (int)this->_modChanLs.size()) {
      this->_modChanSl = i;
      this->log(2, L"ModHub("+this->_title+L") Select Mod Channel",
                L"\""+this->_modChanLs[_modChanSl]->title()+L"\".");
    } else {
      return false;
    }
  } else {
    this->_modChanSl = -1;
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModHub::modChanSelect(const wstring& uuid)
{
  for(size_t i = 0; i < this->_modChanLs.size(); ++i) {
    if(uuid == this->_modChanLs[i]->uuid()) {
      this->_modChanSl = i;
      this->log(2, L"ModHub("+this->_title+L") Select Mod Channel",
                L"\""+this->_modChanLs[_modChanSl]->title()+L"\".");
      return true;
    }
  }

  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
int OmModHub::modChanIdx(const wstring& uuid)
{
  for(size_t i = 0; i < this->_modChanLs.size(); ++i) {
    if(uuid == this->_modChanLs[i]->uuid()) {
      return i;
    }
  }

  return -1;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModHub::modChanCreate(const wstring& title, const wstring& install, const wstring& library, const wstring& backup)
{
  // this theoretically can't happen, but we check to be sure
  if(!this->isValid()) {
    this->_error = L"Mod Hub is empty.";
    this->log(0, L"ModHub(<anonymous>) Create Mod Channel", this->_error);
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
      this->log(0, L"ModHub("+this->_title+L") Create Mod Channel", this->_error);
      return false;
    }
  } else {
    this->log(1, L"ModHub("+this->_title+L") Create Mod Channel",
              Om_errExists(L"Mod Channel home",chn_home));
  }

  // compose Mod Channel definition file name
  wstring chn_def_path = chn_home + L"\\" + title;
  chn_def_path += L"."; chn_def_path += OMM_CHN_DEF_FILE_EXT;

  // check whether definition file already exists and delete it
  if(Om_isFile(chn_def_path)) {

    this->log(1, L"ModHub("+this->_title+L") Create Mod Channel",
              Om_errExists(L"Definition file",chn_def_path));

    int result = Om_fileDelete(chn_def_path);
    if(result != 0) {
      this->_error = Om_errDelete(L"Old definition file", chn_def_path, result);
      this->log(0, L"ModHub("+this->_title+L") Create Mod Channel", this->_error);
      return false;
    }
  }

  // initialize new definition file
  OmConfig chn_def;
  if(!chn_def.init(chn_def_path, OMM_XMAGIC_CHN)) {
    this->_error = Om_errInit(L"Definition file", chn_def_path, chn_def.lastErrorStr());
    this->log(0, L"ModHub("+this->_title+L") Create Mod Channel", this->_error);
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
  chn_xml.child(L"title").setAttr(L"index", static_cast<int>(this->_modChanLs.size()));

  // define installation destination folder in definition file
  chn_xml.addChild(L"install").setContent(install);

  // checks whether we have custom Backup folder
  if(backup.empty()) {
    // Create the default backup sub-folder
    Om_dirCreate(chn_home + L"\\Backup");
  } else {
    // check whether custom Library folder exists
    if(!Om_isDir(backup)) {
      this->log(1, L"ModHub("+this->_title+L") Create Mod Channel",
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
      this->log(1, L"ModHub("+this->_title+L") Create Mod Channel",
                Om_errIsDir(L"Custom Library folder", library));
    }
    // add custom library in definition
    chn_xml.addChild(L"library").setContent(library);
  }

  // save and close definition file
  chn_def.save();
  chn_def.close();

  this->log(2, L"ModHub("+this->_title+L") Create Mod Channel", L"Mod Channel \""+title+L")\" created.");

  // load the newly created Mod Channel
  OmModChan* pModChan = new OmModChan(this);
  pModChan->open(chn_def_path);
  this->_modChanLs.push_back(pModChan);

  // sort locations by index
  this->modChanSort();

  // select the last added location
  this->modChanSelect(this->_modChanLs.size() - 1);

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModHub::modChanDelete(unsigned id)
{
  if(id >= this->_modChanLs.size())
    return false;

  OmModChan* pModChan = this->_modChanLs[id];

  if(pModChan->bckHasData()) {
    this->_error = L"Aborted: Still have backup data to be restored.";
    this->log(0, L"ModHub("+this->_title+L") Delete Mod Channel", this->_error);
    return false;
  }

  bool has_error = false;

  // keep Mod Channel paths
  wstring chn_name = pModChan->title();
  wstring chn_home = pModChan->home();
  wstring chn_path = pModChan->path();

  // close Mod Channel
  pModChan->close();

  // remove the default backup folder
  wstring bck_path = chn_home + L"\\Backup";
  if(Om_isDir(bck_path)) {
    // this will fails if folder not empty, this is intended
    int result = Om_dirDelete(bck_path);
    if(result != 0) {
      this->log(1, L"ModHub("+this->_title+L") Delete Mod Channel",
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
        this->log(1, L"ModHub("+this->_title+L") Delete Mod Channel",
                  Om_errDelete(L"Library folder", lib_path, result));
      }
    } else {
      this->log(1, L"ModHub("+this->_title+L") Delete Mod Channel",
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
      this->log(1, L"ModHub("+this->_title+L") Delete Mod Channel", this->_error);
      has_error = true; //< this is considered as a real error
    }
  }

  // check if location home folder is empty, if yes, we delete it
  if(Om_isDirEmpty(chn_home)) {
    int result = Om_dirDelete(chn_home);
    if(result != 0) {
      this->_error = Om_errDelete(L"Home folder", chn_home, result);
      this->log(1, L"ModHub("+this->_title+L") Delete Mod Channel", this->_error);
      has_error = true; //< this is considered as a real error
    }
  } else {
    this->log(1, L"ModHub("+this->_title+L") Delete Mod Channel",
              L"Non-empty home folder \""+chn_home+L"\" will not be deleted");
  }

  this->log(2, L"ModHub("+this->_title+L") Delete Mod Channel",
            L"Mod Channel \""+chn_name+L"\" deleted.");

  // delete object
  delete pModChan;

  // remove from list
  this->_modChanLs.erase(this->_modChanLs.begin()+id);

  // update locations order indexing
  for(size_t i = 0; i < this->_modChanLs.size(); ++i) {
    this->_modChanLs[i]->setIndex(i);
  }

  // sort Mod Channels by index
  this->modChanSort();

  // select the first available location
  this->modChanSelect(0);

  return !has_error;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModHub::batSort()
{
  if(this->_batLs.size() > 1)
    sort(this->_batLs.begin(), this->_batLs.end(), __bat_sort_index_fn);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmBatch* OmModHub::batAdd(const wstring& title)
{
  // compose path using title and context home
  wstring path = this->_home + L"\\";
  path += title; path += L"."; path += OMM_BAT_DEF_FILE_EXT;

  // Create new batch object
  OmBatch* pBat = new OmBatch(this);
  pBat->init(path, title, this->_batLs.size());
  this->_batLs.push_back(pBat);

  this->log(2, L"ModHub("+this->_title+L") Create Batch", L"Batch \""+title+L"\" created.");

  // sort Batches by index
  this->batSort();

  return pBat;
}



///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModHub::batRem(unsigned id)
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
        this->log(1, L"ModHub("+this->_title+L") Delete Batch", this->_error);
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

    this->log(2, L"ModHub("+this->_title+L") Delete Batch", L"Installation Batch \""+bat_title+L"\" deleted.");

    return true;
  }

  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModHub::setBatQuietMode(bool enable)
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
void OmModHub::log(unsigned level, const wstring& head, const wstring& detail)
{
  wstring log_str = L"Manager:: "; log_str.append(head);

  this->_manager->log(level, log_str, detail);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModHub::_migrate(const wstring& path)
{
  // Function to migrate Mod Hub structure and definitions files from old standard
  // to new standard
  int result;
  OmXmlDoc xmldoc;
  vector<wstring> file_ls;
  vector<wstring> dirs_ls;
  vector<wstring> delt_ls;
  vector<OmXmlNode> xmlnode_ls;

  wstring hub_dir = path + L"\\";
  wstring spt_dir = path + L"\\_Scripts\\";
  wstring spt_mig, chn_mig, hub_mig;

  // Check whether we got an OMC file, meaning migration is required
  Om_lsFileFiltered(&file_ls, hub_dir, L"*.omc", true);
  if(!file_ls.size())
    return true;

  // Here we go for Mod Hub set migration
  this->log(2, L"ModHub("+hub_dir+L") Migration",
            L"Found old fashion Mod Hub that must be migrated.");

  // We first create the new '_Scripts' folder, this will also tell us
  // that Mod Hub directory is writable
  if(!Om_isDir(spt_dir)) {
    // Create "Scripts" directory, fail silently
    result = Om_dirCreate(spt_dir);
    if(result != 0) {
      this->_error = Om_errCreate(L"Script directory", spt_dir, result);
      this->log(1, L"ModHub("+hub_dir+L") Migration", this->_error);
      return false;
    }
  }

  // We now migrate Scripts files, xml 'magic' node is modified and file
  // saved into the new dedicated '_Script' folder

  // Search for Scripts within Mod Hub home directory
  file_ls.clear();
  Om_lsFileFiltered(&file_ls, hub_dir, L"*.omb", true);



  // Modify and save each script file
  for(size_t i = 0; i < file_ls.size(); ++i) {

    // Load definition and change the 'magic' node
    xmldoc.load(file_ls[i]);
    OmXmlNode magic = xmldoc.child(L"Open_Mod_Manager_Batch");
    if(!magic.empty()) magic.setName(OMM_XMAGIC_SPT);

    // Rename all Scripts <location> by <modchan>
    xmldoc.children(xmlnode_ls, L"location");
    for(size_t i = 0; i < xmlnode_ls.size(); ++i)
      xmlnode_ls[i].setName(L"modchan");

    // Save file with new name at new location
    spt_mig = spt_dir + Om_getNamePart(file_ls[i]) + L".xml";
    if(!xmldoc.save(spt_mig)) {
      this->_error = Om_errCreate(L"Script file", spt_mig, result);
      this->log(1, L"ModHub("+hub_dir+L") Migration", this->_error);
      return false;
    }

    xmldoc.clear();

    // add old file to be deleted
    delt_ls.push_back(file_ls[i]);
  }

  // Now migrate Mod Channel(s)
  Om_lsDir(&dirs_ls, hub_dir, false);

  for(size_t i = 0; i < dirs_ls.size(); ++i) {

    // check for presence of old standard Mod Channel definition file
    file_ls.clear();
    Om_lsFileFiltered(&file_ls, hub_dir+dirs_ls[i], L"*.oml", true);

    // Parse the first file found
    if(file_ls.size()) {

      // Load definition and change the 'magic' node
      xmldoc.load(file_ls[0]);
      OmXmlNode magic = xmldoc.child(L"Open_Mod_Manager_Location");
      if(!magic.empty()) magic.setName(OMM_XMAGIC_CHN);

      chn_mig = Om_getDirPart(file_ls[0]) + L"\\ModChan.xml";

      // Save file with new name
      if(!xmldoc.save(chn_mig)) {
        this->_error = Om_errCreate(L"Mod Channel file", spt_mig, result);
        this->log(1, L"ModHub("+hub_dir+L") Migration", this->_error);
        return false;
      }

      xmldoc.clear();

      // add old file to be deleted
      delt_ls.push_back(file_ls[0]);
    }
  }

  // Finally migrate the Mod Hub definition file
  file_ls.clear();
  Om_lsFileFiltered(&file_ls, hub_dir, L"*.omc", true);

  if(file_ls.size()) {

    // Load definition and change the 'magic' node
    xmldoc.load(file_ls[0]);
    OmXmlNode magic = xmldoc.child(L"Open_Mod_Manager_Context");
    if(!magic.empty()) magic.setName(OMM_XMAGIC_HUB);

    hub_mig = hub_dir + L"ModHub.xml";

    // Save file with new name
    if(!xmldoc.save(hub_mig)) {
      this->_error = Om_errCreate(L"Mod Hub file", hub_mig, result);
      this->log(1, L"ModHub("+hub_dir+L") Migration", this->_error);
      return false;
    }

    xmldoc.clear();

    // add old file to be deleted
    delt_ls.push_back(file_ls[0]);
  }

  // Here we go for Mod Hub set migration
  this->log(2, L"ModHub("+hub_dir+L") Migration",
            L"Migration appear successful, cleaning old data.");

  for(size_t i = 0; i < delt_ls.size(); ++i) {
    result = Om_fileDelete(delt_ls[i]);
    if(result != 0) {
      this->_error = Om_errDelete(L"Old file", delt_ls[i], result);
      this->log(1, L"ModHub("+hub_dir+L") Migration", this->_error);
      return false;
    }
  }

  return true;
}
