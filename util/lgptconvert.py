import xml.etree.ElementTree as ET
import math
import sys

song_num = 8
song_step = 128
chain_num = 0x80
chain_step = 16
phrase_num = 0x80
phrase_step = 16
table_num = 0x20
table_step = 16
sample_num = 16
midi_num = 16

cmd_conv = {
    '41525047': '00',
    '43484E4C': '01',
    '43525348': '02',
    '43525356': '03',
    '444C4159': '04',
    '4453504C': '05',
    '454E445F': '06',
    '4556415F': '07',
    '4556425F': '08',
    '4556444E': '09',
    '45564C46': '0A',
    '45564C53': '0B',
    '45565253': '0C',
    '45565254': '0D',
    '45565354': '0E',
    '45565550': '0F',
    '4642414D': '10',
    '46424D58': '11',
    '4642544E': '12',
    '46425455': '13',
    '46435554': '14',
    '46494D4F': '15',
    '464C5452': '16',
    '464D4958': '17',
    '464E544E': '18',
    '46524553': '19',
    '47524F56': '1A',
    '484F5020': '1B',
    '494E5450': '1C',
    '49525447': '1D',
    '4B494C4C': '1E',
    '4C454741': '1F',
    '4C454E47': '20',
    '4C4C454E': '21',
    '4C4D4F44': '22',
    '4C4F4144': '23',
    '4C504F46': '24',
    '4C535441': '25',
    '4D444343': '26',
    '4D445047': '27',
    '4D494449': '28',
    '4D535452': '29',
    '50414E20': '2A',
    '50414E5F': '2B',
    '5046494E': '2C',
    '2D2D2D2D': '2D',
    '504C4F46': '2E',
    '50524749': '2F',
    '50544348': '30',
    '50555247': '31',
    '51554954': '32',
    '524F4F54': '33',
    '52545247': '34',
    '53415645': '35',
    '534D504C': '36',
    '53544F50': '37',
    '53545254': '38',
    '53565053': '39',
    '5441424C': '3A',
    '54424544': '3B',
    '54424C41': '3C',
    '54454D50': '3D',
    '544D504F': '3E',
    '54525350': '3F',
    '54535152': '40',
    '54544150': '41',
    '564C444E': '42',
    '564C5550': '43',
    '564F4C4D': '44',
    '57524150': '45',
    }

def get_used_inst(tree: ET.ElementTree) -> set:
    inst_data = tree.find("SONG/INSTRUMENTS")
    inst_used = set()
    for data in inst_data:
        if not data.attrib:
            for n in range(0, len(data.text), 2):
                if data.text[n:n+2] != "FF":
                    inst_used.add(data.text[n:n+2])
    return inst_used

def get_defined_tables(tree: ET.ElementTree) -> set:
    table_data = tree.find("TABLES")
    table_defined = set()
    for table in table_data:
        if "ID" in table.attrib:
            table_defined.add(table.attrib["ID"])
    return table_defined

def get_used_tables(tree: ET.ElementTree) -> set:
    table_used = set()
    # tables used in instrument bank
    instbank_data = tree.find("INSTRUMENTBANK")
    inst_used = get_used_inst(tree)

    # Table IDs are in hex but instruments reference them in dec
    for data in instbank_data:
        # if the instrument is not used, I don't care about it's table
        if "ID" in data.attrib:
            if not data.attrib["ID"] in inst_used:
                continue
        for param in data:
            if "NAME" in param.attrib and "VALUE" in param.attrib:
                if param.attrib["NAME"] == "table":
                    if param.attrib["VALUE"] != "-1":
                        table_used.add("{:02X}".format(int(param.attrib["VALUE"])))

    # Analyze tables in commands
    for num in ["1", "2"]:
        command_data = tree.find(f"SONG/COMMAND{num}")
        command_concat = ""
        for data in command_data:
            if not data.attrib:
                command_concat += data.text
            else:
                if "VALUE" in data.attrib and "LENGTH" in data.attrib:
                    command_concat += "{:02X}".format(int(data.attrib["VALUE"])) * int(data.attrib["LENGTH"])
        table_loc = []
        for n in range(0, len(command_concat), 8):
            if command_concat[n:n+8] == "5441424C":
                table_loc.append(n)

        param_data = tree.find(f"SONG/PARAM{num}")
        param_concat = ""
        for data in param_data:
            if not data.attrib:
                param_concat += data.text
            else:
                if "VALUE" in data.attrib and "LENGTH" in data.attrib:
                    param_concat += "{:02X}".format(int(data.attrib["VALUE"])) * int(data.attrib["LENGTH"])
        for loc in table_loc:
            # Table def in first byte
            table_used.add(param_concat[int(loc/2):int(loc/2)+2])

    # Tables can also be defined in tables
    # TODO

    return table_used

def analyze(tree: ET.ElementTree):
    result = {}
    # Analyze song
    song_data = tree.find("SONG/SONG")
    song_len = [0] * 8
    for data in song_data:
        if not data.attrib:
            # there is data here
            for n in range(0, len(data.text), 2):
                if data.text[n:n+2] != "FF":
                    song_len[int(n/2) % 8] += 1
        else:
            break
    result["song_num"] = sum([v>0 for v in song_len])
    result["song_step"] = max(song_len)

    # Analyze chains
    chain_data = tree.find("SONG/CHAINS")
    chain_len = [0] * 256
    next_chain = 0
    for r, data in enumerate(chain_data):
        if not data.attrib:
            for n in range(0, len(data.text), 2):
                chain_num = math.floor((n + (r * 128)) / 32)
                if chain_num < next_chain:
                    continue
                if data.text[n:n+2] != "FF":
                    chain_len[chain_num] += 1
                else:
                    # consider chain finished if we hit FF
                    next_chain = chain_num + 1
    result["chain_num"] = sum([v>0 for v in chain_len])
    result["chain_step"] = max(chain_len)
    # we can ignore transposes since they should align with phrases, no meaning of transposes without it's phrase

    # Analyze Phrase
    phrase_data = tree.find("SONG/NOTES")
    notes_used = [False] * 256
    for r, data in enumerate(phrase_data):
        if not data.attrib:
            for n in range(0, len(data.text), 2):
                phrase_num = math.floor((n + (r * 128)) / 32)
                if data.text[n:n+2] != "FF":
                    notes_used[phrase_num] = True
    result["phrase_num"] = sum(notes_used)
    # we are not going to reduce this
    result["phrase_step"] = 16

    # Analyze instruments
    inst_used = get_used_inst(tree)

    instbank_data = tree.find("INSTRUMENTBANK")
    sample_inst_defined = set()
    midi_inst_defined = set()
    for inst in instbank_data:
        if "ID" in inst.attrib:
            # quick hack, sample instruments have more parameters
            if len(inst) > 10:
                sample_inst_defined.add(inst.attrib["ID"])
            else:
                midi_inst_defined.add(inst.attrib["ID"])
            
    result["sample_num"] = len(inst_used.intersection(sample_inst_defined))
    result["midi_num"] = len(inst_used.intersection(midi_inst_defined))

    # Analyze Tables
    table_defined = get_defined_tables(tree)
    table_used = get_used_tables(tree)

    result["table_num"] = len(table_used.intersection(table_defined))
    # we are not going to reduce this
    result["table_step"] = 16

    return result

def convert(lgptsav: ET.ElementTree) -> str:
    # We assume the data fits at this point, so we don't make any checks

    root = ET.Element("PICOTRACKER")
    # Project
    lgpt_project_data = lgptsav.find("PROJECT")
    # TODO: should we set some version?
    project = ET.SubElement(root, "PROJECT", attrib=lgpt_project_data.attrib)
    for param in lgpt_project_data:
        if param.attrib["NAME"] == "midi":
            ET.SubElement(project, "PARAMETER", attrib={"NAME": "midi", "VALUE": "MIDI OUT 1"})
        else:
            ET.SubElement(project, "PARAMETER", attrib=param.attrib)

    # Tables conversion
    # we assess tables before to know the conversions
    table_used = get_used_tables(lgptsav)
    table_map = {}
    count = 0
    for table in sorted(table_used):
        table_map[table] = "{:02X}".format(count)
        count += 1

    # Song
    song = ET.SubElement(root, "SONG")
    ## Song
    lgpt_song_data = lgptsav.find("SONG/SONG")

    song_song = ET.SubElement(song, "SONG")
    # TODO: support converting to less songs

    # We assume the default datalength of 64 to be true always
    # each line length == 64 stores 8 steps for 8 songs
    data_count = 0
    for data in lgpt_song_data:
        if data_count < 16:
            if not data.attrib:
                song_song_data = ET.SubElement(song_song, "DATA")
                song_song_data.text = data.text
            else:
                ET.SubElement(song_song, "DATA", attrib=data.attrib)
        data_count += 1

    ## Chains
    # each line length == 64 stores 4 chains 16 steps in length
    # TODO: compress/reorder chains?
    lgpt_chain_data = lgptsav.find("SONG/CHAINS")

    song_chain = ET.SubElement(song, "CHAINS")
    data_count = 0
    for data in lgpt_chain_data:
        if data_count < 32:
            if not data.attrib:
                song_chain_data = ET.SubElement(song_chain, "DATA")
                song_chain_data.text = data.text
            else:
                ET.SubElement(song_chain, "DATA", attrib=data.attrib)
        data_count += 1
    ## Transposes
    # each line length == 64 stores 4 transposes 16 steps in length
    lgpt_transp_data = lgptsav.find("SONG/TRANSPOSES")

    song_transp = ET.SubElement(song, "TRANSPOSES")
    data_count = 0
    for data in lgpt_transp_data:
        if data_count < 32:
            if not data.attrib:
                song_transp_data = ET.SubElement(song_transp, "DATA")
                song_transp_data.text = data.text
            else:
                ET.SubElement(song_transp, "DATA", attrib=data.attrib)
        data_count += 1
    ## Notes
    # each line length == 64 stores 4 phrases length 16
    lgpt_note_data = lgptsav.find("SONG/NOTES")

    song_note = ET.SubElement(song, "NOTES")
    data_count = 0
    for data in lgpt_note_data:
        if data_count < 32:
            if not data.attrib:
                song_note_data = ET.SubElement(song_note, "DATA")
                song_note_data.text = data.text
            else:
                ET.SubElement(song_note, "DATA", attrib=data.attrib)
        data_count += 1

    ## Instruments
    # each line length == 64 stores 4 phrases instruments length 16
    lgpt_inst_data = lgptsav.find("SONG/INSTRUMENTS")

    song_inst = ET.SubElement(song, "INSTRUMENTS")

    # We first "compress" the instruments in order to fit the first 16 and depending on what's actually used
    inst_used = set()
    for data in lgpt_inst_data:
        if not data.attrib:
            for n in range(0, len(data.text), 2):
                if data.text[n:n+2] != "FF":
                    inst_used.add(data.text[n:n+2])
    inst_map = {}
    count = 0
    for inst in sorted(inst_used):
        inst_dec = int(f"0x{inst}", 0)
        if inst_dec < 128:
            inst_map[inst] = "{:02X}".format(count)
            count += 1
        else:
            count = 16
            inst_map[inst] = "{:02X}".format(count)
            count += 1
    # midi
    for n in range(16):
        inst = "{:02X}".format(n + 128)
        new_inst = "{:02X}".format(n + sample_num)
        inst_map[inst] = new_inst

    data_count = 0
    for data in lgpt_inst_data:
        if data_count < 32:
            if not data.attrib:
                text = ""
                for n in range(0, len(data.text), 2):
                    if data.text[n:n+2] != "FF":
                        text += inst_map[data.text[n:n+2]]
                    else:
                        text += "FF"
                song_inst_data = ET.SubElement(song_inst, "DATA")
                song_inst_data.text = text
            else:
                ET.SubElement(song_inst, "DATA", attrib=data.attrib)
        data_count += 1
    
    for num in ["1", "2"]:
        ## Command1/2
        # each line length == 64 stores 1 phrase command length 16
        lgpt_cmd_data = lgptsav.find(f"SONG/COMMAND{num}")

        song_cmd = ET.SubElement(song, f"COMMAND{num}")

        data_count = 0
        # concat all data
        data_concat = ""
        empty = "2D" * 64
        for data in lgpt_cmd_data:
            if data_count < 128:
                if not data.attrib:
                    data_concat += data.text
                else:
                    data_concat += empty
            data_count += 1

        table_loc = []
        # iterate over the whole string at intervals equal to 64 * 4 of data (each data == 2 bytes)
        for n in range(0, len(data_concat), 512):
            line  = data_concat[n:n+512]
            if line == empty * 4:
                ET.SubElement(song_cmd, "DATA", attrib={"VALUE": "45", "LENGTH": "64"})
            else:
                song_cmd_data = ET.SubElement(song_cmd, "DATA")
                text = ""
                for i in range(0, len(line), 8):
                    if line[i:i+8] == "5441424C":
                        table_loc.append(n + i)
                    text += cmd_conv[line[i:i+8]]
                song_cmd_data.text = text

        ## Param1/2
        lgpt_param_data = lgptsav.find(f"SONG/PARAM{num}")
        song_param = ET.SubElement(song, f"PARAM{num}")

        # concat all data
        data_count = 0
        data_concat = ""
        empty = "00" * 64
        for data in lgpt_param_data:
            if data_count < 64:
                if not data.attrib:
                    data_concat += data.text
                else:
                    data_concat += empty
            data_count += 1

        table_pos = -1
        if table_loc:
            table_pos = table_loc.pop(0)
        for n in range(0, len(data_concat), 128):
            line  = data_concat[n:n+128]
            if line == empty:
                ET.SubElement(song_param, "DATA", attrib={"VALUE": "0", "LENGTH": "64"})
            else:
                song_param_data = ET.SubElement(song_param, "DATA")
                text = ""
                for i in range(0, len(line), 4):
                    if n + i == table_pos / 2:
                        text += table_map[line[i:i+2]] + "00"
                        if table_loc:
                            table_pos = table_loc.pop(0)
                        else:
                            table_pos = -1
                    else:
                        text += line[i:i+4]
                song_param_data.text = text
        
    # InstrumentBank
    ib = ET.SubElement(root, "INSTRUMENTBANK")
#    inst_params = ["note length", "sample", "volume", "interpol", "crush", "crushdrive", "root note", "pan", "filter cut", "filter res", "start", "loopmode", "loopstart", "end", "table", "table automation"]

    lgpt_ib_data = lgptsav.find("INSTRUMENTBANK")
    ## Instrument ID=XX
    for inst in lgpt_ib_data:
        inst_id = inst.attrib["ID"]
        if inst_id not in inst_used:
            continue
        ib_inst = ET.SubElement(ib, "INSTRUMENT", attrib={"ID": inst_map[inst_id]})
    ### Param XX
        for param in inst:
 #           if param.attrib["NAME"] in inst_params:
            # do table conversion. Table values in dec on instruments
            if param.attrib["NAME"] == "table" and param.attrib["VALUE"] != "-1":
                table_hex = "{:02X}".format(int(param.attrib["VALUE"]))
                new_table_dec = int(f"0x{table_map[table_hex]}", 0)
                ET.SubElement(ib_inst, "PARAM", attrib={"NAME": "table", "VALUE": f"{new_table_dec}"})
            else:
                ET.SubElement(ib_inst, "PARAM", attrib=param.attrib)
    # Tables
    ## Table ID=XX
    ### CMD1
    ### PARAM1
    ### CMD2
    ### PARAM2
    ### CMD3
    ### PARAM3
    tables = ET.SubElement(root, "TABLES")
    lgpt_tables_data = lgptsav.find("TABLES")

    for table in lgpt_tables_data:
        if table.attrib["ID"] not in table_used:
            continue
        table_node = ET.SubElement(tables, "TABLE", attrib={"ID": table_map[table.attrib["ID"]]})
        for elem in table:
            if elem.tag.startswith("CMD"):
                # process CMD DATA
                cmd_node = ET.SubElement(table_node, elem.tag)
                if elem[0].attrib:
                    # no text, so empty node
                    ET.SubElement(cmd_node, "DATA", attrib={"VALUE": "45", "LENGTH": "16"})
                else:
                    data_node = ET.SubElement(cmd_node, "DATA")
                    src_text = elem[0].text
                    text = ""
                    for n in range(0, len(src_text), 8):
                        text += cmd_conv[src_text[n:n+8]]
                    data_node.text = text
            else:
                param_node = ET.SubElement(table_node, elem.tag)
                if elem[0].attrib:
                    ET.SubElement(param_node, "DATA", attrib=elem[0].attrib)
                else:
                    data_node = ET.SubElement(param_node, "DATA")
                    data_node.text = elem[0].text

    ET.indent(root, space="    ", level=0)
    return ET.tostring(root, encoding='utf8', method='xml')

def main(args):
    tree = ET.parse(args[1]);
    if tree.getroot().tag != "LITTLEGPTRACKER":
        print("Not a LGPT file!\n")
        return

    result = analyze(tree)
    if result["song_step"] <= song_step and result["chain_num"] <= chain_num and result["phrase_num"] <= phrase_num and result["sample_num"] <= sample_num and result["midi_num"] <= midi_num and result["table_num"] <= table_num:
        convert(tree)
        print(convert(tree).decode("utf-8"))
    else:
        print("Cannot convert, project doesn't fit picoTracker")

if __name__ == "__main__":
    # command line args: file, chains, phrases, tables, sample insts, midi insts
    if len(sys.argv) < 2:
        print("Need file name!\n")
    else:
        main(sys.argv)
