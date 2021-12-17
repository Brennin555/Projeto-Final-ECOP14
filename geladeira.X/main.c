/*
 * File:   main.c
 * Author: Brenno de Oliveira da Rosa - 2021029935
 *
 * Created on 22 de Novembro de 2021, 15:27
 */

#include "config.h"
#include "bits.h"
#include "lcd.h"
#include "ssd.h"
#include "delay.h"
#include "adc.h"
#include "io.h"
#include "so.h"
#include "pwm.h"
#include "keypad.h"
#include <pic18f4520.h>


#define L0 0x80
#define L1 0xC0
#define CLR 0x01
#define ON 0x0F

//declaração das variaveis
unsigned int tecla = 16;
unsigned long int m1 = 0;

float vtempo;
char timer = 0; // Indica se o timer é para a geladeira ou o freezer
char cronOn = 0; // Indica se o tempo já foi definido

char aberta = 0; //Indica se a geladeira está aberta


char nMensagem = 0; //Mensagens que aparecem no visor

char potencia = 7; //variável com o último valor de potencia
char potenciaG = 7; //potencia da geladeira
char potenciaF = 7; //potencia do freezer
char potenciaAux = 0; // verifica qual potencia que irá aumentar
char turbo = 9;

char potenciaUpdate = 0;  //Verifica se o valor da potência foi alterado

char tempot = 0;

void msg(void) // Função responsável por enviar os menus ao LCD
{
    switch (nMensagem) 
    {
        case 0:
            lcdString("INICIO");
            break;
        case 1:
            lcdString(" Potencia\n Geladeira");
            break;
        case 2:
            lcdString(" Potencia\n Freezer");
            break;
        case 3:
            lcdString(" Timer\n Geladeira");
            break;
        case 4:
            lcdString(" Timer\n Freezer");
            break;
        case 5:
            lcdString(" Defina o tempo:\n(potenciometro)");
            break;
        case 6:
            lcdString(" Potencia:\n MINIMO");
            break;
        case 7:
            lcdString(" Potencia:\n MEDIO");
            break;
        case 8:
            lcdString(" Potencia:\n MAXIMO");
            break;
        default:
            lcdString("");
            break;
    }
}

void geladeiraAberta(void) // Função responsável por acionar o buzzer enquanto a porta estiver aberta
{
    if (aberta == 1) {
        pwmSet(100);
        tempo(250);
        pwmSet(0);
        tempoS(2);
    }
}

void main(void) 
{

    //inicialização dos componentes
    soInit();
    adcInit();
    ssdInit();
    lcdInit();
    kpInit();
    pwmInit();

    for (;;) 
    {
        lcdCommand(L0);
        kpDebounce();

        msg();
        geladeiraAberta();

        if (kpRead() != tecla) 
        {
            tecla = kpRead();

            if (bitTst(tecla, 0)) // SAI DA TELA INICIAL E ABRE O MENU     
            {        
                if (aberta == 1) 
                {
                    aberta = 0;  // DESLIGA O ALARME DE "PORTA ABERTA" DA GELADEIRA
                } 
                else
                {
                    lcdCommand(CLR);
                    if (nMensagem == 0) 
                    {
                        nMensagem = 1;
                    } 
                    else
                    { 
                        
                        if (nMensagem > 5) 
                        {
                            if (potenciaAux == 0) 
                            {
                                potenciaG = potencia; // Volta para o valor anterior da potência
                            } 
                            else 
                            {
                                potenciaF = potencia; // Volta para o valor anterior da potencia
                            }
                            nMensagem = 0;
                        }
                        
                    }
                }
            }

            if (bitTst(tecla, 1)) { // VOLTA PARA A OPÇÃO ANTERIOR
                if ((nMensagem != 0)&&(nMensagem <= 4)) 
                {
                    lcdCommand(0x01);
                    nMensagem = nMensagem - 1;
                    if (nMensagem <= 0) 
                    {
                        lcdCommand(0x01);
                        nMensagem = 4;
                    }
                }
                if (nMensagem > 5) 
                {
                    lcdCommand(0x01);
                    nMensagem = nMensagem - 1;
                    if (nMensagem < 6) 
                    {
                        lcdCommand(0x01);
                        nMensagem = 8;
                    }

                    if (potenciaAux == 0) 
                    { 
                        potenciaG = nMensagem;  //Atualiza o valor da potencia de acordo com a opção do visor
                    } else 
                    {
                        potenciaF = nMensagem;  //Atualiza o valor da potencia de acordo com a opção do visor
                    }
                }
            }

            if (bitTst(tecla, 3)) //AVANÇA PARA A OPÇÃO SEGUINTE
            { 
                if ((nMensagem != 0)&&(nMensagem < 5)) 
                {
                    lcdCommand(0x01);
                    nMensagem = nMensagem + 1;
                    if ((nMensagem > 4)) 
                    {
                        lcdCommand(0x01);
                        nMensagem = 1;
                    }
                }
                if (nMensagem >= 6) 
                {
                    lcdCommand(0x01);
                    nMensagem = nMensagem + 1;
                    if ((nMensagem > 8)) 
                    {
                        lcdCommand(0x01);
                        nMensagem = 6;
                    }
                    if (potenciaAux == 0) 
                    {  
                        potenciaG = nMensagem;  //Atualiza o valor da potencia de acordo com a opção do visor
                    } 
                    else 
                    {
                        potenciaF = nMensagem;  //Atualiza o valor da potencia de acordo com a opção do visor
                    }

                }
            }

            if (bitTst(tecla, 2)) // SELECIONA A OPÇÃO ATUAL
            { 
                lcdCommand(0x80);
                lcdCommand(0x01);

                if ((potenciaUpdate == 1)&&(nMensagem>5)) //VERIFICA SE A POTENCIA DA GELADEIRA OU FREEZER JÁ FOI ALTERADA
                { 
                    lcdCommand(0x01);
                    nMensagem = 0;
                    potenciaUpdate = 0;
                }

                if ((nMensagem == 1) || (nMensagem == 2) || (nMensagem > 5)) // MINIMO MÉDIO E MÁXIMO
                { 
                    if (nMensagem == 1) 
                    {
                        potenciaAux = 0;
                    } 
                    else if (nMensagem == 2) 
                    {
                        potenciaAux = 1;
                    }


                    nMensagem = 7; //MUDA A TELA PARA AS OPÇÕES DE MÍNIMO MÉDIO E MÁXIMO


                    if (nMensagem > 5) 
                    {
                        lcdCommand(CLR);
                        potenciaUpdate = 1;

                        if (potenciaAux == 0) 
                        {
                            potencia = potenciaG;
                        } 
                        else 
                        {
                            potencia = potenciaF;
                        }
                        //
                    }
                }

                if (nMensagem == 5) // verifica se o timer já foi regulado
                { 
                    if (timer == 1) 
                    {
                        potenciaG = 9;
                    } 
                    else 
                    {
                        potenciaF = 9;
                    }

                    cronOn = 2;
                    nMensagem = 0;
                    timer = 0;
                }

                if ((nMensagem == 3) || (nMensagem == 4)) //verifica se é solicitado o timer
                { 
                    lcdCommand(CLR);
                    if (nMensagem == 3) 
                    {
                        timer = 1;
                    } 
                    else 
                    {
                        timer = 2;
                    }
                    cronOn = 1;
                    nMensagem = 5;

                }
            }
            if (bitTst(tecla, 4)) // SIMULA A GELADEIRA ABERTA
            { 
                aberta = 1;
            }

        }

        ssdDigit(potenciaG - 5, 0); //INDICA A POTENCIA DA GELADEIRA
        ssdDigit(potenciaF - 5, 1); //INDICA A POTENCIA DO FREEZER

        switch (cronOn) { //(timer != 0) {
            case 0:
                ssdDigit(0, 2); //INDICA A POTENCIA DA GELADEIRA
                ssdDigit(0, 3); //INDICA A POTENCIA DO FREEZER
                break;
            case 1:
                m1 = adcRead(0)*9;

                ssdDigit(((m1 / 1000) % 10), 2);
                ssdDigit(((m1 / 100) % 10), 3);
                break;

            case 2:
                ssdDigit(((m1 / 1000) % 10), 2);
                ssdDigit(((m1 / 100) % 10), 3);
                m1--;

                if (m1 == 0) 
                {
                    pwmSet(150);
                    tempoS(3);
                    pwmSet(0);
                    cronOn = 0;

                    if (potenciaG == 9) 
                    {
                        potenciaG = 7;      // volta a geladeira para a potência média ao acabar zerar o timer
                    } 
                    else if (potenciaF == 9) 
                    {
                        potenciaF = 7;      // volta o freezer para a potência média ao acabar zerar o timer
                    }
                }
                break;
        }

        ssdUpdate();
        for (vtempo = 0; vtempo < 125; vtempo++); //tempo tava em 100
    }
    return;
}
