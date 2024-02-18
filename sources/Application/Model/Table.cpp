#include "Table.h"
#include "Application/Instruments/CommandList.h"
#include "Application/Utils/HexBuffers.h"
#include "Application/Utils/char.h"
#include "Song.h"
#include "System/System/System.h"

Table::Table() { Reset(); };

void Table::Reset() {
  for (int i = 0; i < TABLE_STEPS; i++) {
    cmd1_[i] = MAKE_FOURCC('-', '-', '-', '-');
    param1_[i] = 0;
    cmd2_[i] = MAKE_FOURCC('-', '-', '-', '-');
    param2_[i] = 0;
    cmd3_[i] = MAKE_FOURCC('-', '-', '-', '-');
    param3_[i] = 0;
  }
};

void Table::Copy(const Table &other) {
  for (int i = 0; i < TABLE_STEPS; i++) {
    cmd1_[i] = *other.cmd1_;
    param1_[i] = *other.param1_;
    cmd2_[i] = *other.cmd2_;
    param2_[i] = *other.param2_;
    cmd3_[i] = *other.cmd3_;
    param3_[i] = *other.param3_;
  }
};

bool Table::IsEmpty() {

  for (int i = 0; i < TABLE_STEPS; i++) {
    if (cmd1_[i] != I_CMD_NONE) {
      return false;
    };
    if (cmd2_[i] != I_CMD_NONE) {
      return false;
    };
    if (cmd3_[i] != I_CMD_NONE) {
      return false;
    };
    if (param1_[i] != 0) {
      return false;
    };
    if (param2_[i] != 0) {
      return false;
    };
    if (param3_[i] != 0) {
      return false;
    };
  }
  return true;
}
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

TableHolder::TableHolder() : Persistent("TABLES") { Reset(); }

void TableHolder::Reset() {
  for (int i = 0; i < SONG_CHANNEL_COUNT; i++) {
    table_[i].Reset();
  }
  for (int i = 0; i < TABLE_COUNT; i++) {
    allocation_[i] = false;
  };
};

Table &TableHolder::GetTable(int table) {
  NAssert((table >= 0) && (table < TABLE_COUNT));
  return table_[table];
}

void TableHolder::SaveContent(tinyxml2::XMLPrinter *printer) {

  char hex[3];
  for (int i = 0; i < TABLE_COUNT; i++) {
    printer->OpenElement("TABLE");
    hex2char(i, hex);
    printer->PushAttribute("ID", hex);

    Table &table = table_[i];
    if (!table.IsEmpty()) {
      //      TiXmlNode *dataNode = node->InsertEndChild(data);
      for (int i = 0; i < 16; i++) {
        table.param1_[i] = Swap16(table.param1_[i]);
        table.param2_[i] = Swap16(table.param2_[i]);
        table.param3_[i] = Swap16(table.param3_[i]);
      }
      saveHexBuffer(printer, "CMD1", table.cmd1_, TABLE_STEPS);
      saveHexBuffer(printer, "PARAM1", table.param1_, TABLE_STEPS);
      saveHexBuffer(printer, "CMD2", table.cmd2_, TABLE_STEPS);
      saveHexBuffer(printer, "PARAM2", table.param2_, TABLE_STEPS);
      saveHexBuffer(printer, "CMD3", table.cmd3_, TABLE_STEPS);
      saveHexBuffer(printer, "PARAM3", table.param3_, TABLE_STEPS);
    }
    printer->CloseElement();
  }
};

void TableHolder::RestoreContent(PersistencyDocument *doc) {

  bool elem = doc->FirstChild();
  while (elem) {
    // Check it is a table
    if (!strcmp(doc->ElemName(), "TABLE")) {
      // Get the table ID
      unsigned char id = '\0';
      bool attr = doc->NextAttribute();
      while (attr) {
        if (!strcmp(doc->attrname_, "ID")) {
          unsigned char b1 = (c2h__(doc->attrval_[0])) << 4;
          unsigned char b2 = c2h__(doc->attrval_[1]);
          id = b1 + b2;
          // found what we wanted
          break;
        }
        attr = doc->NextAttribute();
      }

      Table &table = table_[id];

      bool subelem = doc->FirstChild();
      while (subelem) {
        if (!strcmp("CMD1", doc->ElemName())) {
          restoreHexBuffer(doc, (unsigned char *)table.cmd1_);
        };
        if (!strcmp("PARAM1", doc->ElemName())) {
          restoreHexBuffer(doc, (unsigned char *)table.param1_);
        };
        if (!strcmp("CMD2", doc->ElemName())) {
          restoreHexBuffer(doc, (unsigned char *)table.cmd2_);
        };
        if (!strcmp("PARAM2", doc->ElemName())) {
          restoreHexBuffer(doc, (unsigned char *)table.param2_);
        };
        if (!strcmp("CMD3", doc->ElemName())) {
          restoreHexBuffer(doc, (unsigned char *)table.cmd3_);
        };
        if (!strcmp("PARAM3", doc->ElemName())) {
          restoreHexBuffer(doc, (unsigned char *)table.param3_);
        };

        for (int i = 0; i < 16; i++) {
          table.param1_[i] = Swap16(table.param1_[i]);
          table.param2_[i] = Swap16(table.param2_[i]);
          table.param3_[i] = Swap16(table.param3_[i]);
        }
        subelem = doc->NextSibling();
      }
      allocation_[id] = !table.IsEmpty();
    }
    elem = doc->NextSibling();
  }
}

void TableHolder::SetUsed(int i) {
  if (i >= TABLE_COUNT) {
    NAssert(i < 128);
  }
  allocation_[i] = true;
};

int TableHolder::GetNext() {
  for (int i = 0; i < TABLE_COUNT; i++) {
    if (!allocation_[i]) {
      if (table_[i].IsEmpty()) {
        allocation_[i] = true;
        return i;
      }
    };
  };
  return NO_MORE_TABLE;
};

int TableHolder::Clone(int table) {
  int target = GetNext();
  if (target != NO_MORE_TABLE) {
    table_[target].Copy(table_[table]);
  };
  return target;
};
