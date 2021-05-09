#
# Carregador das definições para o ATmega Detonator
#
# As definições são lidas do arquivo chips.json e carregadas
# na EEProm do Detonator através da serial
#
# Assume que o arquivo json foi codificado corretamente,
# nenhuma consistência ou reformatação é feita
#
import json
import binascii
import serial

# Serial onde o Detonator está conectado
PORTA = 'COM3'

# le o json
with open("chips.json", "r") as arq_def:
    definicoes = json.load(arq_def)
chips = definicoes["chips"]
bootloaders = definicoes["bootloaders"]

# determina os indices dos bootloaders
boot_indices = {}
for i in range(0, len(bootloaders)):
    boot_indices[bootloaders[i]["id"]] = i

# determina o espaço ocupado pelas definições dos chips e bootloaders
tam_chipentry = 3+4+17+1+2+5
n_chips = len(chips)
tam_chips = tam_chipentry * (n_chips+1)
tam_bootentry = 17+4+2+1+2
n_boots = len(bootloaders)
tam_boots = tam_bootentry * n_boots
addr_bootdef = (1+tam_chips+255) // 256
addr_bootcode = addr_bootdef + ((tam_boots+255) // 256)
print ('{:d} chips, {:d} bytes por chip, {:d} bytes no total'.format(n_chips, tam_chipentry, tam_chips))
print ('{:d} bootloaders, {:d} bytes por bootloader, {:d} bytes no total'.format(n_boots, tam_bootentry, tam_boots))
print ('Endereco definicoes dos bootloaders: {:02X}00'.format(addr_bootdef))
print ('Endereco codigo dos bootloaders: {:02X}00'.format(addr_bootcode))
print ()

# abre e configura a serial
print ('Aguarde o Detonator reiniciar e selecione "Carrega Config"')
ser = serial.Serial(PORTA, 115200, timeout=2)

# aguarda Detonator pronto
while True:
    ser.write('*'.encode("ascii"))
    resp = ser.read()
    if (len(resp) > 0) and (resp[0] == 0x2E):
        break

# envia comando e aguarda Detonator
def envia(cmd):
    print(cmd)
    ser.write(cmd.encode("ascii"))
    while True:
        resp = ser.read()
        if (len(resp) > 0) and (resp[0] == 0x2E):
            break

# grava o endereço inicial da tabela de bootloaders
addr = 0
cmd = ':{:04X} {:02X} {:02X}'.format(addr, 1, addr_bootdef)
envia(cmd)
addr = addr+1

# Codifica um nome em hexadecimal
def codNome(nome):
    x = ''.join("{:02X}".format(c) for c in nome.encode("ascii"))
    return (x+"0000000000000000000000000000000000")[0:34]

# Codifica um valor de 16 bits em hexadecimal little-endian
def codInt16(val):
    return "{:02X}{:02X}".format(val & 0xFF, val >> 8)

# grava as definições dos chips
for chip in chips:
    loaders = ''
    for id in chip['bootloaders']:
        loaders = loaders + '{:02X}'.format(boot_indices[id])
    while len(loaders) < 10:
        loaders = loaders + "FF"
    cmd = ':{:04X} {:02X} {:s}{:s}{:s}{:02X}{:s}{:s}'.\
           format(addr, tam_chipentry, chip['id'], \
           chip['fusesFab'], codNome(chip['nome']), \
           chip['pageSize'], codInt16(chip['numPages']), loaders)
    envia (cmd)
    addr = addr + tam_chipentry

# grava marca de fim dos chips
cmd = ':{:04X} {:02X} {:s}{:s}{:s}{:02X}{:04X}{:s}'.\
    format(addr, tam_chipentry, 'FFFFFF', \
    'FFFFFFFF', codNome('*FIM*'), \
    0, 0, 'FFFFFFFFFF')
envia (cmd)
addr = addr + tam_chipentry

# grava as definições dos bootloaders
addr = addr_bootdef*256
addr_code = addr_bootcode
for boot in bootloaders:
    addrBoot = boot['addrBoot'][2:4]+boot['addrBoot'][0:2]
    cmd = ':{:04X} {:02X} {:s}{:s}{:s}{:02X}{:s}'.\
           format(addr, tam_bootentry, codNome(boot['nome']), \
           boot['fuses'], addrBoot, addr_code, codInt16(boot['tamanho']))
    envia (cmd)
    addr = addr + tam_bootentry
    addr_code = addr_code + ((boot['tamanho'] + 255) // 256)

# grava o código dos bootloaders
addr = addr_bootcode*256
for boot in bootloaders:
    print (hex(addr)+' '+boot["code"])
    fim = addr + boot["tamanho"]
    startBoot = int(boot["addrBoot"], base=16)
    hexfile = open(boot["code"], 'r', encoding='ascii')
    lines = hexfile.readlines()
    hexfile.close()
    for line in lines:
        if line[7:9] != '00':
            continue
        ender = int(line[3:7], base=16) - startBoot + addr
        if ender >= fim:
            continue
        cmd = ':{:04X} {:s} {:s}'.format(ender, line[1:3], line[9:len(line)-3])
        envia (cmd)
    addr = fim
    
# encerra a carga
envia ('!')
