import serial
import glob
import sys

ports = glob.glob('/dev/ttyACM*')
if not ports:
    print("Erro: Nenhuma sonda Pico H encontrada em /dev/ttyACM*")
    sys.exit(1)
port = ports[0]
 
try:
    ser = serial.Serial(port, 115200, timeout=5)
except Exception as e:
    print(f"Erro ao abrir a porta {port}: {e}")
    sys.exit(1)

# Monta o pacote binário do Projeto Dolos
magic = 0xAA
cmd_start_session = 0x04
arg = 0x0 
checksum = magic ^ cmd_start_session ^ arg
packet = bytes([magic, cmd_start_session, arg, checksum])

print(f"Enviando comando para a sonda ({port}): {[hex(x) for x in packet]}")
ser.write(packet)
ser.flush()
print("Comando enviado, aguardando telemetria e resposta...\n")

try:
    while True:
        raw_line = ser.readline()
        if not raw_line:
            print("Tempo limite esgotado (Timeout) aguardando o IDCODE.")
            break
            
        response = raw_line.decode(errors="ignore").strip()
        print(f"[Sonda] -> {response}")

        if "DBG:IDCODE_OK" in response:
            print("\n🎉 SUCESSO: SWD conectado e IDCODE lido corretamente!")
            break
            
        elif "DBG:IDCODE_FAIL" in response:
            print("\n❌ ERRO (S004): Falha na conexão SWD ou IDCODE incorreto.")
            break

except KeyboardInterrupt:
    print("\nComunicação encerrada pelo usuário.")
except Exception as e:
    print(f"\nErro durante a leitura: {e}")
finally:
    ser.close()
    print("Porta serial fechada.")
