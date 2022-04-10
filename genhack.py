import json

f = open("bypass.json", "r")
f2 = open("hacks.txt", "w")

Json = json.loads(f.read())
#print(Json["hacks"][3]["opcodes"])


def formatBytes(hackCode):
    # E9 79 06 00 00
    result = ""
    hackCode = hackCode.split(" ")
    for number in hackCode:
        number = "0x"+number+", "
        result = result + number
    return result


    # 0xE9б, 0x79, 0x06, 0x00, 0x00


def formatOpcodes(Json, number):
    result_on = ""
    result_off = ""
    for x in Json["hacks"][number]["opcodes"]:
        third_string = 'WriteBytes((void*)(gd::base + {}), {{{}}});\n'.format(x["addr"], formatBytes(x["on"]))
        fifth_string = 'WriteBytes((void*)(gd::base + {}), {{{}}});\n'.format(x["addr"], formatBytes(x["off"]))
        result_on = result_on + third_string
        result_off = result_off + fifth_string
    print(result_on)
    print(result_off)



def formatCommand(hackName, hackDesc, hackAddr, hackOn, hackOff):
    


    first_string = 'ImGui::Checkbox("{}", &{}Enabled);'.format(hackName, hackName)
    one = 'if (ImGui::IsItemHovered())'
    two = 'ImGui::SetTooltip("{}");'.format(hackDesc)
    second_string = 'if ({}Enabled){{'.format(hackName)
    third_string = '\tWriteBytes((void*)(gd::base + {}), {{{}}});'.format(hackAddr, formatBytes(hackOn))
    forth_string = '} else {'
    fifth_string = '\tWriteBytes((void*)(gd::base + {}), {{{}}});'.format(hackAddr, formatBytes(hackOff))
    final = '}\n'
    print(first_string)
    print(second_string)
    print(third_string)
    print(forth_string)
    print(fifth_string)
    print(final)
    f2.write(first_string+"\n"+one+"\n"+two+"\n"+second_string+"\n"+third_string+"\n"+forth_string+"\n"+fifth_string+"\n"+final)

#for hack in Json["hacks"]:
    #formatCommand((hack["name"]), hack["desc"], hack["opcodes"][0]["addr"], hack["opcodes"][0]["on"], hack["opcodes"][0]["off"])

#print(formatBytes("F3 0F 2C C0 85 C0 0F 4F C8 B8 64 00 00 00 3B C8 0F 4F C8 8B 87 C0 03 00 00 51 68 30 32 69 00 8B B0 04 01 00 00"))


formatOpcodes(Json, 14)

f.close()
f2.close()