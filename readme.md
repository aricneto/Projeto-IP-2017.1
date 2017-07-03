# Projeto IP 2017.1 - Revenge of the Giant Crab
Este é um projeto de um jogo multiplayer para a cadeira de Introdução a Programação do CIn - UFPE.
# Compilando
Para compilar, é necessário ter a biblioteca ncurses:  
`sudo apt-get install libncurses5-dev`

Tendo a biblioteca, digite:  
`make server` e, em outro terminal, `make client`

# Sobre o jogo 
O objetivo do jogo é destruir o caranguejo gigante. No entanto, o caranguejo é protegido por um muro e por seus fiéis soldados (Berseks, runners e casters). 
 
## Número de jogadores  
São permitidos até 4 jogadores na partida.
 
## Controle  
A movimentação do jogador é feita através das setas (cima, baixo, esquerda e direita).  
O ataque é feito corpo-a-corpo. Para isso, você deve utilizar as teclas W, A, S, D para atacar para cima, esquerda, baixo e direita, respectivamente. 
 
## Inimigos
O jogo contém 4 tipos de inimigos:  
* Runners - Os Runners são os que existem em maior quantidade. Eles também são os com menor poder de ataque e com menos vidas.
* Casters - Os Casters são os médios.
* Bersek - Os Bersekers estão em menor quantidade. Eles têm maior poder de ataque e maior resistência.

Os 3 tipos de monstros acima tem suas características alteradas durante o jogo. A medida que novas hordas de monstros chegam, eles chegam com ataques maiores, mais vida e em maior quantidade.

### O Caranguejo Gigante  
O caranguejo gigante é o último obstáculo para os guerreiros. Ele não consegue atacar, mas tem poder de vida extremamente alto.
 
## Etapas do Jogo
O guerreiro não consegue chegar ao caranguejo devido ao muro. Os monstros seguem o guerreiro para atacá-lo. O guerreiro deve atacar os monstros para matá-los.
Destruídas as duas primeiras hordas de monstros, o muro é destruído, dando acesso do guerreiro ao caranguejo gigante.
Vencendo o caranguejo, o jogador ganha a partida.
 
 
 

