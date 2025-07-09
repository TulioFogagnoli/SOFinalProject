import socket
import time

# --- Configurações ---
# O endereço IP que você configurou no seu STM32 (em freertos.c)
STM32_IP = "192.168.18.177"  
# A porta que você definiu no seu STM32 (no código da thread RX)
STM32_PORT = 7

def main():
    """
    Função principal para conectar ao STM32, enviar comandos de duty cycle
    e receber os valores do ADC.
    """
    print(f"Tentando conectar ao STM32 em {STM32_IP}:{STM32_PORT}...")

    try:
        # Cria um socket TCP/IP
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            # Tenta conectar ao servidor (seu STM32)
            s.connect((STM32_IP, STM32_PORT))
            print("Conectado com sucesso!")
            print("Digite um valor de duty cycle (0-100) e pressione Enter.")
            print("Digite 'sair' para terminar.")

            # Configura o socket para não bloquear indefinidamente ao receber dados
            s.settimeout(1.0) 

            while True:
                # Pede ao usuário para inserir um novo valor de duty cycle
                duty_cycle_str = input("\nNovo Duty Cycle (0-100): ")

                if duty_cycle_str.lower() == 'sair':
                    break
                
                try:
                    # Tenta converter o valor para um número
                    duty_cycle_int = int(duty_cycle_str)
                    if not 0 <= duty_cycle_int <= 100:
                        print("Erro: Por favor, insira um número entre 0 e 100.")
                        continue

                    # Envia o valor para o STM32, codificado como bytes
                    s.sendall(str(duty_cycle_int).encode('utf-8'))
                    print(f"Enviado: {duty_cycle_int}%")

                    # Pausa para dar tempo ao STM32 de responder
                    time.sleep(0.5)

                    # Tenta ler a resposta (valor do ADC)
                    try:
                        resposta = s.recv(1024)
                        if resposta:
                            print(f"Resposta do STM32 (ADC): {resposta.decode('utf-8').strip()}")
                        else:
                            # Se não houver resposta, o servidor provavelmente desconectou
                            print("Conexão fechada pelo servidor.")
                            break
                    except socket.timeout:
                        print("Nenhuma resposta do STM32 dentro de 1 segundo.")

                except ValueError:
                    print("Erro: Entrada inválida. Por favor, insira um número.")

    except ConnectionRefusedError:
        print("\nErro: Conexão recusada. Verifique se:")
        print("1. O STM32 está ligado e executando o programa.")
        print("2. O módulo Ethernet está corretamente ligado à rede.")
        print("3. O endereço IP no script está correto.")
        print("4. Não há um firewall a bloquear a conexão.")
    except Exception as e:
        print(f"\nOcorreu um erro inesperado: {e}")

    print("\nScript terminado.")

if __name__ == "__main__":
    main()