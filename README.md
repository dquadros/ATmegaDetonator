# ATmegaDetonator
Projeto de um programador paralelo ("alta tensão") para o ATmega8/168/328

Os diretórios tem os seguintes conteúdos:
* **Apaga**: Teste de apagamento da Flash
* **Caixa**: Desenho da caixa para corte com Laser Cutter
* **Chips**: Definição dos modelos suportados, imagens dos bootloaders e programa Python para carga no *ATmega Detonator*
* **Demo_PCF8274**: Teste do uso do PCF8274 como expansão de entrada e saída
* **Detonator**: Aplicação propriamente dita
* **Identificação**: Teste de leitura dos bytes de identificação do ATmega
* **LeFuses**: Teste de leitura dos *fuses* (bytes de configuração do ATmega)
* **LeGravaFlash**: Teste de gravação e leitura da Flash do ATmega
* **Teste 2432**: Teste de acesso à EEProm I2C 2432
* **Teste Conexões**: Programa para testar a montagem do circuito

Observações:
* Foram feitos testes com os modelos ATmega328P, ATmega328 e ATmega168. Outros modelos podem exigir ajustes além das configurações em chips.json
* O programa Python para carga da definição dos chips e bootloaders é bastante simplório, pressupondo que o arquivo json foi criado corretamente com todas informações necessárias (e nos formatos adequados)
* Detalhes sobre o projeto e o código podem ser encontrados no meu blog https://dqsoft.blogspot.com
