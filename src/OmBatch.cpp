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

#include "OmBaseApp.h"

#include "OmUtilFs.h"
#include "OmUtilWin.h"
#include "OmUtilHsh.h"

#include "OmManager.h"
#include "OmContext.h"
#include "OmModChan.h"
#include "OmPackage.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmBatch.h"


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmBatch::OmBatch() :
  _context(nullptr), _config(), _path(), _uuid(), _title(), _index(0),
  _instOnly(false), _error()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmBatch::OmBatch(OmContext* pCtx) :
  _context(pCtx), _config(), _path(), _uuid(), _title(), _index(0),
  _instOnly(false), _error()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmBatch::~OmBatch()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmBatch::open(const wstring& path)
{
  // clear the instance
  this->close();

  // try to open and parse the XML file
  if(!this->_config.open(path, OMM_XMAGIC_BAT)) {
    this->_error = L"Error loading Batch definition: ";
    this->_error += this->_config.lastErrorStr();
    this->log(0, wstring(L"Batch(")+path+L") Open", this->_error);
    return false;
  }

  // check for the presence of <uuid> entry
  if(!this->_config.xml().hasChild(L"uuid")) {
    this->_error = L"Parse error: <uuid> node missing, malformed Batch definition file.";
    this->log(0, L"Batch("+path+L") Open", this->_error);
    return false;
  }

  // check for the presence of <title> entry
  if(!this->_config.xml().hasChild(L"title")) {
    this->_error = L"Parse error: <title> node missing, malformed Batch definition file.";
    this->log(0, L"Batch("+path+L") Open", this->_error);
    return false;
  }

  this->_path = path;
  this->_uuid = this->_config.xml().child(L"uuid").content();
  this->_title = this->_config.xml().child(L"title").content();
  this->_index = this->_config.xml().child(L"title").attrAsInt(L"index");

  // check for <options>
  if(this->_config.xml().hasChild(L"options")) {
    this->_instOnly = this->_config.xml().child(L"options").attrAsInt(L"installonly");
  }

  this->log(2, L"Batch("+this->_title+L") Open", L"Definition parsed.");

  // Launch repair
  this->repair();

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmBatch::init(const wstring& path, const wstring& title, unsigned index)
{
  // clear the instance
  this->close();

  // try to open and parse the XML file
  if(!this->_config.init(path, OMM_XMAGIC_BAT)) {
    this->_error = L"Error initializing Batch definition: ";
    this->_error += this->_config.lastErrorStr();
    this->log(0, wstring(L"Batch(")+path+L") Open", this->_error);
    return false;
  }

  // store file path
  this->_path = path;

  // Set batch values
  this->_uuid = Om_genUUID();
  this->_title = title;
  this->_index = index;

  // set uuid, title and index
  this->_config.xml().addChild(L"uuid").setContent(this->_uuid);
  OmXmlNode xml_title = this->_config.xml().addChild(L"title");
  xml_title.setContent(this->_title);
  xml_title.setAttr(L"index", static_cast<int>(this->_index));

  // create the <options> node
  this->_config.xml().addChild(L"options").setAttr(L"installonly", 0);

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmBatch::setTitle(const wstring& title)
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
void OmBatch::setIndex(unsigned index)
{
  if(this->_config.valid()) {

    this->_index = index;

    if(this->_config.xml().hasChild(L"title")) {
      this->_config.xml().child(L"title").setAttr(L"index", static_cast<int>(index));
    }

    this->_config.save();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmBatch::setInstallOnly(bool enable)
{
  if(this->_config.valid()) {

    this->_instOnly = enable;

    OmXmlNode xml_options;
    if(this->_config.xml().hasChild(L"options")) {
      xml_options = this->_config.xml().child(L"options");
    } else {
      xml_options = this->_config.xml().addChild(L"options");
    }

    xml_options.setAttr(L"installonly", static_cast<int>(enable));

    this->_config.save();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t OmBatch::chnCount()
{
  if(this->_config.valid()) {
    return this->_config.xml().childCount(L"location");
  }

  return 0;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
wstring OmBatch::locUuid(unsigned i)
{
  wstring uuid;

  if(this->_config.valid()) {
    OmXmlNode xml_chn = this->_config.xml().child(L"location", i);
    uuid = xml_chn.attrAsString(L"uuid");
  }

  return uuid;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmBatch::locDiscard(const wstring& uuid)
{
  bool result = false;

  if(this->_config.valid()) {

    // get the proper <location> node.
    OmXmlNode xml_chn = this->_config.xml().child(L"location", L"uuid", uuid);

    // if no <location> with uuid was found, create it
    if(!xml_chn.empty()) {
      this->_config.xml().remChild(xml_chn);
      result = true;
    }

    // Write definition file
    if(this->_path.size())
      this->_config.save();
  }

  return result;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmBatch::instClear(const OmModChan* pChn)
{
  if(this->_config.valid()) {

    // get the proper <location> node.
    OmXmlNode xml_chn = this->_config.xml().child(L"location", L"uuid", pChn->uuid());

    // if no <location> with uuid was found, create it
    if(!xml_chn.empty()) {
      this->_config.xml().remChild(xml_chn);
    }

    // Write definition file
    if(this->_path.size())
      this->_config.save();
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmBatch::instAdd(const OmModChan* pChn, const OmPackage* pPkg)
{
  if(this->_config.valid()) {

    // get the proper <location> node.
    OmXmlNode xml_chn = this->_config.xml().child(L"location", L"uuid", pChn->uuid());

    // if no <location> with uuid was found, create it
    if(xml_chn.empty()) {
      xml_chn = this->_config.xml().addChild(L"location");
      xml_chn.setAttr(L"uuid", pChn->uuid());
    }

    // add <install> entry if not already exists
    if(!xml_chn.hasChild(L"install", L"ident", pPkg->ident())) {
       OmXmlNode xml_ins = xml_chn.addChild(L"install");
       xml_ins.setAttr(L"ident", pPkg->ident());
       xml_ins.setAttr(L"hash", pPkg->hash());
    }

    // Write definition file
    if(this->_path.size())
      this->_config.save();
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmBatch::instRem(const OmModChan* pChn, const wstring ident)
{
  if(this->_config.valid()) {

    bool result = false;

    // get the proper <location> node.
    OmXmlNode xml_chn = this->_config.xml().child(L"location", L"uuid", pChn->uuid());

    // if no <location> with uuid was found, return
    if(xml_chn.empty()) {
      return false;
    }

    // get <install> node list
    vector<OmXmlNode> xml_ins_ls;
    xml_chn.children(xml_ins_ls, L"install");

    // check for <install> entry and remove it
    if(xml_chn.hasChild(L"install", L"ident", ident)) {
      result = xml_chn.remChild(xml_chn.child(L"install", L"ident", ident));
    }

    // Write definition file
    if(this->_path.size())
      this->_config.save();

    return result;
  }

  return false;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t OmBatch::instSize(const OmModChan* pChn)
{
  if(this->_config.valid()) {

    // get the proper <location> node.
    OmXmlNode xml_chn = this->_config.xml().child(L"location", L"uuid", pChn->uuid());

    // if found, return count of <install> child nodes.
    if(!xml_chn.empty())
      return xml_chn.childCount(L"install");
  }

  return 0;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmPackage* OmBatch::instGet(const OmModChan* pChn, unsigned i)
{
  if(this->_config.valid()) {

    // get the proper <location> node.
    OmXmlNode xml_chn = this->_config.xml().child(L"location", L"uuid", pChn->uuid());

    // if found, return count of <install> child nodes.
    if(!xml_chn.empty()) {

      OmXmlNode xml_ins = xml_chn.child(L"install", i);
      OmPackage* pPkg;

      // first try and rely on package hash value
      if(xml_ins.hasAttr(L"hash")) {
        if((pPkg = pChn->pkgFind(xml_ins.attrAsUint64(L"hash"))))
          return pPkg;
      }

      // then try with identity
      if(xml_ins.hasAttr(L"ident")) {
        if((pPkg = pChn->pkgFind(xml_ins.attrAsString(L"ident"))))
          return pPkg;
      }
    }
  }

  return nullptr;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t OmBatch::instGetList(const OmModChan* pChn, vector<OmPackage*>& pkg_ls)
{
  if(this->_config.valid()) {

    // get the proper <location> node.
    OmXmlNode xml_chn = this->_config.xml().child(L"location", L"uuid", pChn->uuid());

    // if found, return count of <install> child nodes.
    if(!xml_chn.empty()) {

      vector<OmXmlNode> xml_ins_ls;
      xml_chn.children(xml_ins_ls, L"install");

      vector<OmXmlNode> xml_dsc_ls; //< discard list

      OmPackage* pPkg;
      for(size_t i = 0; i < xml_ins_ls.size(); ++i) {

        // first try and rely on package hash value
        if(xml_ins_ls[i].hasAttr(L"hash")) {
          if((pPkg = pChn->pkgFind(xml_ins_ls[i].attrAsUint64(L"hash")))) {
            pkg_ls.push_back(pPkg); continue;
          }
        }

        // then try with identity
        if(xml_ins_ls[i].hasAttr(L"ident")) {
          if((pPkg = pChn->pkgFind(xml_ins_ls[i].attrAsString(L"ident")))) {
            pkg_ls.push_back(pPkg); continue;
          }
        }

      }

      return pkg_ls.size();
    }
  }

  return 0;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmBatch::repair()
{
  if(!this->_context) {
    this->log(1, L"Batch("+this->_title+L") Repair", L"No Context bound, aborting.");
    return false;
  }

  if(!this->_config.valid()) {
    this->log(1, L"Batch("+this->_title+L") Repair", L"Definition not initialized, aborting.");
    return false;
  }

  // usefull strings for job
  wstring uuid, ident;

  // list for nodes to discard
  vector<OmXmlNode> xml_dis_ls;
  // <location> nodes list
  vector<OmXmlNode> xml_chn_ls;

  // remove Mod Channel broken references
  this->_config.xml().children(xml_chn_ls, L"location");

  // build the discard list
  for(size_t i = 0; i < xml_chn_ls.size(); ++i) {
    uuid = xml_chn_ls[i].attrAsString(L"uuid");
    if(!this->_context->chnGet(uuid)) {
      xml_dis_ls.push_back(xml_chn_ls[i]);
      this->log(2, L"Batch("+this->_title+L") Repair", L"Discard Mod Channel reference: "+uuid);
    }
  }

  if(xml_dis_ls.size()) {
    for(size_t i = 0; i < xml_dis_ls.size(); ++i) {
      this->_config.xml().remChild(xml_dis_ls[i]);
    }
  }

  // clear and load new Mod Channel list
  xml_chn_ls.clear();
  this->_config.xml().children(xml_chn_ls, L"location");

  OmModChan* pChn;
  OmPackage* pPkg;

  vector<OmXmlNode> xml_ins_ls;

  // remove Package broken references
  for(size_t i = 0; i < xml_chn_ls.size(); ++i) {

    // get Mod Channel
    pChn = this->_context->chnGet(xml_chn_ls[i].attrAsString(L"uuid"));

    // get install list for this location
    xml_ins_ls.clear();
    xml_chn_ls[i].children(xml_ins_ls, L"install");

    // clear the discard list
    xml_dis_ls.clear();

    // build the discard list or repair reference
    for(size_t j = 0; j < xml_ins_ls.size(); ++j) {

      pPkg = nullptr;

      if(xml_ins_ls[j].hasAttr(L"hash")) {

        uint64_t hash = xml_ins_ls[j].attrAsUint64(L"hash");

        if((pPkg = pChn->pkgFind(hash))) {

            // add missing ident reference
            if(!xml_ins_ls[j].hasAttr(L"ident")) {
              xml_ins_ls[j].setAttr(L"ident", pPkg->ident());
              this->log(2, L"Batch("+this->_title+L") Repair", L"Repair Package reference: "+pPkg->ident());
            }

        } else {
          xml_dis_ls.push_back(xml_ins_ls[j]);
          this->log(2, L"Batch("+this->_title+L") Repair", L"Discard Package reference: "+Om_uint64ToStr(hash));
        }

      } else if(xml_ins_ls[j].hasAttr(L"ident")) {

        ident = xml_ins_ls[j].attrAsString(L"ident");

        if((pPkg = pChn->pkgFind(ident))) {
            // add missing hash reference
            if(!xml_ins_ls[j].hasAttr(L"hash")) {
              xml_ins_ls[j].setAttr(L"hash", pPkg->hash());
              this->log(2, L"Batch("+this->_title+L") Repair", L"Repair Package reference: "+Om_uint64ToStr(pPkg->hash()));
            }
        } else {
          xml_dis_ls.push_back(xml_ins_ls[j]);
          this->log(2, L"Batch("+this->_title+L") Repair", L"Discard Package reference: "+ident);
        }

      } else {
        // discard invalid entry
        xml_dis_ls.push_back(xml_ins_ls[j]);
        this->log(2, L"Batch("+this->_title+L") Repair", L"Discard invalid Package entry");
      }

    }

    // remove nodes
    if(xml_dis_ls.size()) {
      for(size_t j = 0; j < xml_dis_ls.size(); ++j) {
        xml_chn_ls[i].remChild(xml_dis_ls[j]);
      }
    }
  }

  // Write definition file
  if(this->_path.size())
    this->_config.save();

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmBatch::rename(const wstring& name)
{
  wstring title = this->_title;
  wstring old_path = this->_path;

  // compose new Batch full path
  wstring new_path = old_path.substr(0, old_path.find_last_of(L"\\") + 1);
  new_path += name; new_path += L"."; new_path += OMM_BAT_DEF_FILE_EXT;

  // close to rename file safely
  this->close();

  // Rename Batch definition file
  int result = Om_fileMove(old_path, new_path);
  if(result != 0) {
    this->_error = L"Unable to rename Batch definition file \"";
    this->_error += old_path + L"\" : ";
    this->_error += Om_getErrorStr(result);
    this->log(0, L"Batch("+title+L")", L"Rename : " + this->_error);
    return false;
  }

  this->log(2, L"Batch("+title+L")", L"Renamed to \"" + new_path + L"\"");

  // Reload Batch
  this->open(new_path);

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmBatch::close()
{
  this->_config.close();
  this->_path.clear();
  this->_title.clear();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmBatch::log(unsigned level, const wstring& head, const wstring& detail)
{
  if(this->_context != nullptr) {

    wstring log_str = L"Context("; log_str.append(this->_context->title());
    log_str.append(L"):: "); log_str.append(head);

    this->_context->log(level, log_str, detail);
  }
}

