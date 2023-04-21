#include <nds.h>
#include <stdio.h>
#include <sprite.h>
#include <TestVert3.h>
#include <Sfondosub1.h>
#include <ufo.h>
#include <BG1.h>
#include <BG2.h>
#include <cctype>
#include <string>
#include <vector>

/*Raccolta documentazione WinGRIT:

Gli sprite per essere usabili devono avere almeno due colori.
Grit a quanto pare considera un unico colore (anche su sfondo trasparente) come una parte da rendere trasparente.

BPP, non sono opzionali: per sprite a 256 colori devono essere 8, 16 colori 4, bitmap 16.
*/

using namespace std;


/* 
Classe Ufo: necessaria in quanto serviva tenere traccia dell'ufo nei due oam, e per gestire l'animazione in maniera generica. Può essere tranquillamente adattata come classe
per un qualsiasi sprite animato. 
*/

class Ufo	{
	int oam;
	int noam;
	int nframe;
	int curframe;
	u16* gfx;
	public:
		Ufo(int, int, int, u16*);
		void setCurrentFrame(int);
		void setGfx(u16*);
		void setNOam(int);
		void setOam(int);
		int getOam();
		int getNOam();
		int getCurrentFrame();
		int getNFrames();
		SpriteEntry* getSprite();
		u16* getGfx();
};

Ufo::Ufo (int oamtype, int oamnumber, int f, u16* vgfx)	//Costruttore, imposta il tipo di oam (2 rappresenta oamSub, 1 rappresenta oamMain), il numero dello sprite in quest'oam, il numero di frame di animazione e l'indirizzo della memoria video allocata alla grafica dello sprite.
{
	oam = oamtype;
	noam = oamnumber;
	nframe = f;
	curframe = 0;
	gfx = vgfx;
}

void Ufo::setCurrentFrame(int frame)
{
	curframe = frame;
}

void Ufo::setGfx(u16* newgfx)
{
	gfx = newgfx;
}

void Ufo::setNOam(int number)
{
	noam = number;
}

void Ufo::setOam(int number)
{
	oam = number;
}

int Ufo::getOam()
{
	return oam;
}

int Ufo::getNOam()
{
	return noam;
}

int Ufo::getCurrentFrame()
{
	return curframe;
}

int Ufo::getNFrames()
{
	return nframe;
}

u16* Ufo::getGfx()
{
	return gfx;
}

SpriteEntry* Ufo::getSprite()	//A seconda dell'oam dove si trova, necessario restituire un indirizzo di memoria diverso per lo sprite.
{
	if(oam == 1)
	{
		return &oamMain.oamMemory[noam];
	}
	else
	{
		return &oamSub.oamMemory[noam];
	}
}

void onlyAnimateCharForSineWave(OamState &oam, int i, int angleCounter) //Serve ad animare correttamente un singolo carattere. Presenta un if ridondante, lasciato perchè indica il ragionamento di separare i movimenti se necessario.
{
	
	SpriteEntry currentletter = oam.oamMemory[i]; //Ottengo i dati della lettera
	s16 SineAngle = (cosLerp(angleCounter * (449.9 + (currentletter.x / 3))) + (currentletter.y * 10)) >> 4; //Creo la funzione che descrive la nuova sinusoide
	if(currentletter.hFlip == true)	//Se il carattere è uno di quelli che è stato marcato da printspaced perchè fuori dal limite:
	{
		oamSetXY(&oam, i, currentletter.x - 1, SineAngle / 10 + 90);
	}
	else
	{
		oamSetXY(&oam, i, currentletter.x - 1, SineAngle / 10 + 90);
	}
	return;
}

void checkSingleChar(OamState &oam, int charnumber, int startingy, int spacing) //Effettua controlli sulla posizione e stato di un singolo carattere
{
	SpriteEntry currentletter = oam.oamMemory[charnumber]; //Ottengo i dati della lettera
	if(charnumber > 0)	//Devo controllare quello precedente, altrimenti 1)Non serve a nulla 2)Non ce l'ho
	{
		if(currentletter.hFlip == true)	//Se è stato marcato:
		{
			SpriteEntry previousletter = oam.oamMemory[charnumber - 1]; //Ottengo i dati della lettera precedente
			if(currentletter.x == 256 && previousletter.x <= 256 - spacing)		//Se la nuova lettera è in posizione per essere mostrata, e quella prima si trova a distanza sufficiente:
			
			{
				oamSetFlip(&oam, charnumber, false, false); //Resetta il marcatore
				oamSetHidden(&oam, charnumber, false); //Rendila visibile
			}
		}
	}
	if(currentletter.x <=0 && currentletter.hFlip == false)				//Cancella (? Questa parte non è ben documentata) la lettera quando esce fuori dallo schermo
	{
	  oamClearSprite(&oam, charnumber);
	}
}

void printCharForSineWave(OamState &oam, int currentoam, u16* lettergfx, int startingx, int startingy, char letter)	//Stampa un singolo carattere
{
	int currentoffset = 0;
	int currentx = startingx + 0;
	
	char currentchar = tolower(letter);		//Supporta solo lettere minuscole al momento per via della spritesheet
	int currentcharvalue = (int) currentchar - 97;	//Ottengo la posizione nell'alfabeto sottraendo il numero ascii di partenza
    currentoffset = 128 * currentcharvalue;	//Calcolo la sua posizione nell'immagine
	if(!(isspace(letter)))	//GESTIONE DEL WHITESPACE 
	{
		oamSet(&oam, currentoam, currentx,startingy, 0, 0, SpriteSize_16x16, SpriteColorFormat_256Color, lettergfx + currentoffset, 0, false, false, false, false, false);
	}
}

vector<int> printSpacedCharMessageForSineWave(OamState &oam, u16* lettergfx, int currentoam, int startingx, int startingy, int spacing, string message)
{
	/*
	Stampa un messaggio invisibile message da animare sull'oam passato, con il font nella spritesheet all'indirizzo lettergfx, partendo dal numero di sprite currentoam, alla posizione startingx e startingy.	
	*/
	
	int lengthofmessage = message.length();
	int currentoffset = 0;
	int currentx = startingx;
	
	vector<int> setOAMS;
	
	for(int i = 0; i <lengthofmessage; i++)
	{
		int currentcharvalue = (int) tolower(message[i]) - 97;
		currentoffset = 128 * currentcharvalue;
		if(!(isspace(message[i])))	//GESTIONE DEL WHITESPACE 
		{
		  oamSet(&oam, currentoam, currentx,startingy, 0, 0, SpriteSize_16x16, SpriteColorFormat_256Color, lettergfx + currentoffset, -1, false, false, false, false, false);
		  setOAMS.push_back(currentoam);
		  //x > 784 compare va in "overflow" e compare ad x = 0.
		if((currentx + spacing) >= 520)
		  {
			  oamSetHidden(&oam, currentoam, true);
			  oamSetFlip(&oam, currentoam, true, false); //Serve a marcarli per checksinglechar, non basta hidden perchè anche clearsprite lo fa.
		  }
		  currentoam+=1;
		  currentx = currentx + spacing;
		}
		else
		{
			currentx = currentx + (spacing / 2);
		}
	}
	return setOAMS;
}

void manualoutputChars(OamState &oam, u16* lettergfx, int currentoam) //Metodo manuale per stampare tutti i caratteri con BMP_1D_128, inserire l'oam, puntatore alla grafica della spritesheet da dmacopy, e l'ultimo numero di sprite settato + 1 (currentoam)
{
	
	/* SPIEGAZIONE FUNZIONAMENTO
	
	Una volta caricato un puntatore all'indirizzo in VRAM, si avrà un certo spazio tra le varie tile, detto offset, a seconda di come si è fatto oaminit. 
	Nel caso di SPRITEMAPPING_1D_128, ogni frame si trova a indirizzoinmemoriavram + (numeroframe * (SpriteSize totale (16 * 16 = 256 in questo caso) /2 )).
	*/

	oamSet(&oamSub, currentoam + 0, 0,0, 0, 0, SpriteSize_16x16, SpriteColorFormat_256Color, lettergfx, 0, false, false, false, false, false);		
	oamSet(&oamSub, currentoam + 1, 10,0, 0, 0, SpriteSize_16x16, SpriteColorFormat_256Color, lettergfx + 128, 0, false, false, false, false, false);		
	oamSet(&oamSub, currentoam + 2, 20,0, 0, 0, SpriteSize_16x16, SpriteColorFormat_256Color, lettergfx + 256, 0, false, false, false, false, false);	
	oamSet(&oamSub, currentoam + 3, 30,0, 0, 0, SpriteSize_16x16, SpriteColorFormat_256Color, lettergfx + 384, 0, false, false, false, false, false);
	oamSet(&oamSub, currentoam + 4, 40,0, 0, 0, SpriteSize_16x16, SpriteColorFormat_256Color, lettergfx + 512, 0, false, false, false, false, false);
	oamSet(&oamSub, currentoam + 5, 50,0, 0, 0, SpriteSize_16x16, SpriteColorFormat_256Color, lettergfx + 640, 0, false, false, false, false, false);
	oamSet(&oamSub, currentoam + 6, 60,0, 0, 0, SpriteSize_16x16, SpriteColorFormat_256Color, lettergfx + 768, 0, false, false, false, false, false);
	oamSet(&oamSub, currentoam + 7, 70,0, 0, 0, SpriteSize_16x16, SpriteColorFormat_256Color, lettergfx + 896, 0, false, false, false, false, false);
	oamSet(&oamSub, currentoam + 8, 80,0, 0, 0, SpriteSize_16x16, SpriteColorFormat_256Color, lettergfx + 1024, 0, false, false, false, false, false);
	oamSet(&oamSub, currentoam + 9, 90,0, 0, 0, SpriteSize_16x16, SpriteColorFormat_256Color, lettergfx + 1152, 0, false, false, false, false, false);
	oamSet(&oamSub, currentoam + 10, 100,0, 0, 0, SpriteSize_16x16, SpriteColorFormat_256Color, lettergfx + 1280, 0, false, false, false, false, false);
	oamSet(&oamSub, currentoam + 11, 110,0, 0, 0, SpriteSize_16x16, SpriteColorFormat_256Color, lettergfx + 1408, 0, false, false, false, false, false);
	oamSet(&oamSub, currentoam + 12, 120,0, 0, 0, SpriteSize_16x16, SpriteColorFormat_256Color, lettergfx + 1536, 0, false, false, false, false, false);
	oamSet(&oamSub, currentoam + 13, 130,0, 0, 0, SpriteSize_16x16, SpriteColorFormat_256Color, lettergfx + 1664, 0, false, false, false, false, false);
	oamSet(&oamSub, currentoam + 14, 140,0, 0, 0, SpriteSize_16x16, SpriteColorFormat_256Color, lettergfx + 1792, 0, false, false, false, false, false);
	oamSet(&oamSub, currentoam + 15, 150,0, 0, 0, SpriteSize_16x16, SpriteColorFormat_256Color, lettergfx + 1920, 0, false, false, false, false, false);
	oamSet(&oamSub, currentoam + 16, 160,0, 0, 0, SpriteSize_16x16, SpriteColorFormat_256Color, lettergfx + 2048, 0, false, false, false, false, false);
	oamSet(&oamSub, currentoam + 17, 170,0, 0, 0, SpriteSize_16x16, SpriteColorFormat_256Color, lettergfx + 2176, 0, false, false, false, false, false);
	oamSet(&oamSub, currentoam + 18, 180,0, 0, 0, SpriteSize_16x16, SpriteColorFormat_256Color, lettergfx + 2304, 0, false, false, false, false, false);
	oamSet(&oamSub, currentoam + 19, 190,0, 0, 0, SpriteSize_16x16, SpriteColorFormat_256Color, lettergfx + 2432, 0, false, false, false, false, false);
	oamSet(&oamSub, currentoam + 20, 200,0, 0, 0, SpriteSize_16x16, SpriteColorFormat_256Color, lettergfx + 2560, 0, false, false, false, false, false);
	oamSet(&oamSub, currentoam + 21, 210,0, 0, 0, SpriteSize_16x16, SpriteColorFormat_256Color, lettergfx + 2688, 0, false, false, false, false, false);
	oamSet(&oamSub, currentoam + 22, 220,0, 0, 0, SpriteSize_16x16, SpriteColorFormat_256Color, lettergfx + 2816, 0, false, false, false, false, false);
	oamSet(&oamSub, currentoam + 23, 230,0, 0, 0, SpriteSize_16x16, SpriteColorFormat_256Color, lettergfx + 2944, 0, false, false, false, false, false);
	oamSet(&oamSub, currentoam + 24, 240,0, 0, 0, SpriteSize_16x16, SpriteColorFormat_256Color, lettergfx + 3072, 0, false, false, false, false, false);
	oamSet(&oamSub, currentoam + 25, 0,20, 0, 0, SpriteSize_16x16, SpriteColorFormat_256Color, lettergfx + 3200, 0, false, false, false, false, false);
	oamSet(&oamSub, currentoam + 26, 10,20, 0, 0, SpriteSize_16x16, SpriteColorFormat_256Color, lettergfx + 3328, 0, false, false, false, false, false);
}
void initVideo()
{
	/*  Mappa la VRAM in modo da mostrare uno sfondo sugli schermi main e sub.

     *  Si mappano le banche A e B alla memoria per sfondi delle schermo main. Otteniamo così
     *  256KB, che è una buona quantità per delle grafiche a 16 bit.
     *
     *  Si mappa la banca C alla memoria sfondi dello schermo sub.
     *
     *  Si mappa la banca F all'LCD. Quest'impostazione è generalmente usata per quando non si usa 
	 *  una banca in particolare.
	 *
	 *	Si mappa la banca E come deposito per gli sprite (64 kb).
     */	
	 vramSetBankA(VRAM_A_MAIN_BG);
	 vramSetBankB(VRAM_B_MAIN_BG);
	 vramSetBankC(VRAM_C_SUB_BG);
	 vramSetBankF(VRAM_F_LCD);
	 vramSetBankH(VRAM_H_LCD);
	 vramSetBankE(VRAM_E_MAIN_SPRITE);
	 vramSetBankD(VRAM_D_SUB_SPRITE);
	 
	 // Imposta la modalità video sullo schermo main.
	 
	 videoSetMode(MODE_5_2D /* Imposta la modalità grafica come Mode 5 */ | DISPLAY_BG2_ACTIVE /* Abilita BG2 come display */ | DISPLAY_BG3_ACTIVE /* Abilita BG3 come display */ | DISPLAY_SPR_ACTIVE /* Abilita gli sprite nel display */ | DISPLAY_SPR_1D /* Abilita gli sprite a tile 1D */ );
	 
	 // Imposta la modalità video sullo schermo sub.
	 
	 videoSetModeSub(MODE_5_2D | DISPLAY_BG2_ACTIVE| DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D);
}

vector<int> initBackgrounds()
{
	/* La funzione initBackgrounds crea e copia in memoria usando dmaCopy gli sfondi.
	Restituisce un vettore di int contenenti i numeri (?) degli sfondi, così da essere usati dopo nella funzione main. */
	
	vector<int> backgrounds;
	
	int bg2 = bgInit(2, BgType_Bmp16, BgSize_B16_256x256, 0,0); //Crea uno sfondo 128x128 con priorità 2, indirizzo in tilemap parte con offset 0, e ne salva l'indirizzo in memoria in bg1.
	dmaCopy(BG2Bitmap, bgGetGfxPtr(bg2), BG2BitmapLen); //Copia la bitmap dello sfondo all'indirizzo in memoria grafica allocato a bg1
	backgrounds.push_back(bg2);
	
	int bg1 = bgInit(3, BgType_Bmp16, BgSize_B16_256x256, 8,0); //Parte da offset tilemap 2, consultare http://cpp4ds.blogspot.com/2013/07/backgrounds.html
	dmaCopy(BG1Bitmap, bgGetGfxPtr(bg1), BG1BitmapLen);
	backgrounds.push_back(bg1);
	
	return backgrounds;
}

int* getRainbowGradientAsArray() //Restituisce un array contenente i colori di un arcobaleno approssimato. Usa argb16 per convertire gli int in valori rgb a 16 bit (NON CORRISPONDE AI COLORI HTML)
{
	static int rainbow[18];
	int c = 0;
	for(int i = 0; i <= 2; i++)
	{
		rainbow[c] = ARGB16(1,255,0 + i * 10,0);
		c++;
	}
	for(int i = 0; i <= 2; i++)
	{
		rainbow[c] = ARGB16(1,220 - i * 10, 60, 0); 
		c++;
	}
	for(int i = 0; i <= 2; i++)
	{
		rainbow[c] = ARGB16(1,0, 255 ,40 + i * 10);
		c++;
	}
	for(int i = 0; i <= 2; i++)
	{
		rainbow[c] = ARGB16(1,0, 220 - i * 10,255);
		c++;
	}
	for(int i = 0; i <= 2; i++)
	{
		rainbow[c] = ARGB16(1,40 + i * 10, 0,255);
		c++;
	}
	for(int i = 0; i <= 2; i++)
	{
		rainbow[c] = ARGB16(1,255, 0,220 - i * 10);
		c++;
	}
	return rainbow;
} 

void showRainbowEffect(int* rainbowTimer, int startingHeight, int numberOfColors, int colorThickness, int length, u16* videoMemory, int* rainbowArray, int rainbowArrayLength, int backgroundColor)
{
	/*Genera ed anima un effetto arcobaleno. Si sovrappone a qualsiasi oggetto che abbia colore backgroundColor. Accetta come parametri un timer, una y di partenza, 
	numero di colori contemporaneamente a schermo, ogni colore quante righe ha contemporaneamente, la lunghezza di una riga di colore, un puntatore alla memoria video 
	dello schermo su cui farlo, un array di colori (in questo caso, gradiente arcobaleno, funziona per ogni gradiente in realtà), la lunghezza di questo array 
	(si può ricavare con i puntatori, però per semplicità va bene lasciare così) ed infine il colore di sfondo da sostituire. */
	
	
	int c = 0;
	for(int i = 1;  i <= numberOfColors; i++)
	{
		for(int z = 0; z <= colorThickness; z++)
		{
			for(int x = 0; x <= length; x++)
			{
				if(videoMemory[x + startingHeight * 256] != backgroundColor) //Colora se abbiamo colori diversi da quello dello sfondo. Se ==, performance inusabile.
				{
					videoMemory[x + startingHeight * 256] = rainbowArray[c]; //Assegna al pixel il colore corrispondente nel gradiente
				}
			}
			startingHeight++;
		}
		c++;
	}
	if(*rainbowTimer == 2)	//Ogni 2 tick, fai shiftare a destra i colori per l'effetto arcobaleno.
	{
		auto var = rainbowArray[0];
		for(c = 0; c <= rainbowArrayLength; c++)
		{
			rainbowArray[c] = rainbowArray[c + 1];
		}
		rainbowArray[rainbowArrayLength] = var;
		*rainbowTimer = 0;
	}
}

void debugColors(u16* videoMemory)
{
	/*Funzione di debug per stampare il colore di ogni pixel sullo schermo con puntatore a memoria video fornito come parametro. ATTENZIONE: RALLENTA TANTISSIMO SE ESEGUITA 
	NEL LOOP PRINCIPALE.*/
	
	/*Per ottenere u16 color, bisogna prima trovare il colore in standard html, poi convertirlo in formato 15 bit gameboy, lo si ottiene in hex,
	convertirlo in decimale.*/
	
	for(int x = 0; x < 256; x++)
	{
		for(int y = 0; y < 192; y++)
		{
			string nadd1text = "Il pixel in posizione ";
			nadd1text += std::to_string(x);
			nadd1text += ",";
			nadd1text += std::to_string(y);
			nadd1text += "ha colore: ";
			nadd1text += std::to_string(videoMemory[x + y * 256]);
			nocashMessage(nadd1text.c_str());
		}
	}
}

int isOffscreen(SpriteEntry sprite)
{
	/* Controlla che la posizione di uno sprite passato come parametro sia all'interno dello schermo che lo contiene. Restituisce 0 in tal caso, altrimenti:
		Se lo sprite è uscito verticalmente, restituisce 1 se da sotto, 2 se da sopra.
		Se è uscito orizzontalmente, restituisce 3.
		Se è fuori schermo sia verticalmente che orizzontalmente, restituisce 4. ATTENZIONE: la funzione di animazione non gestisce il caso 4 al momento.
		Questi valori sono inoltre approssimazioni, in quanto x e y non sempre corrispondono nelle due oam, e sono perciò presenti bug in funzioni che usano questa.
	*/
	if((sprite.x >= 256 || sprite.x < 0) && (sprite.y >= 192 || sprite.y < 0))
	{
		return 4;
	}
	else
	{
		if(sprite.x >= 256 || sprite.x < 0)
		{
			return 3;
		}
		else
		{
			if(sprite.y >= 192 || sprite.y < 0)
			{
				if(sprite.y > 200)	//Uscito da sotto
				{
					return 1;
				}
				else				//Uscito da sopra
				{
					return 2;
				}
			}
			else return 0;
		}
	}
}

void debugMSG(string message, int value)	//Funzione semplice di debug, stampa sulla console NO$GBA una coppia stringa-integer.
{
	message += ": ";
	message += std::to_string(value);
	nocashMessage(message.c_str());
}

void debugPosition(OamState &oam, int oamNum, string name)
{
	/*Funzione di debug per stampare la posizione x e y dello sprite nella oam passata e con certo numero di oam nella console NOSGBA, e se esso è fuori schermo. Il nome dello sprite stampato nella console è il parametro name.*/
	SpriteEntry sprite = oam.oamMemory[oamNum];	//Prende lo sprite dalla memoria oam passata
	string nadd1text = "Posizione di " + name;
	nadd1text += std::to_string(sprite.x);
	nadd1text += ",";
	nadd1text += std::to_string(sprite.y);
	nadd1text += ", risultato controllo fuorischermo: ";
	nadd1text += std::to_string(isOffscreen(sprite));
	nocashMessage(nadd1text.c_str());
}

void changeUfoScreen(Ufo *ufo)
{
	/* 
	Questa funzione si occupa di cambiare lo schermo in cui si trova l'ufo: nel momento in cui esso esce dallo schermo, viene chiamata questa funzione.
	è necessario rimuovere prima dall'oam corrente, successivamente inserirlo in una nuova posizione sull'altra oam. 
	NB: Nell'implementazione corrente, i valori di x e y vengono sovrascritti dalla funzione di animazione.
	*/
	
	SpriteEntry* ufoSprite = ufo->getSprite();
	int newx = ufoSprite->x;
	//int palettenum = ufo->getSprite()->palette; Serve nel caso di sprite a 16 colori, per salvare il numero di palette impostato, da 1 a 16. 
	int newy = 0;
	if(ufo->getOam() == 1)	//Schermo main
	{
		oamClearSprite(&oamMain, ufo->getNOam());	//Rimuove lo sprite dall'oam
		u16* ufogfxsub = oamAllocateGfx(&oamSub, SpriteSize_32x32, SpriteColorFormat_256Color);	//Alloca memoria
		dmaCopy(ufoTiles, ufogfxsub, ufoTilesLen);	//Copia tile dello sprite in memoria
		oamSet(&oamSub, ufo->getNOam(), newx, newy, 0, 0, SpriteSize_32x32, SpriteColorFormat_256Color, ufogfxsub + (512 * ufo->getCurrentFrame()), 0, false, false, false, false, false); //Versione 256 colori
		//oamSet(&oamSub, ufo->getNOam(), newx, newy, 0, 0, SpriteSize_32x32, SpriteColorFormat_256Color, ufogfxsub + (256 * ufo->getCurrentFrame()), 0, false, false, false, false, false); //Versione 16 colori
		ufo->setOam(2);	//Tiene conto del cambio di OAM
		ufo->setGfx(ufogfxsub);	//Tiene conto del cambio di indirizzo in memoria della grafica
		//ufo->getSprite()->palette = palettenum; Serve per sprite che usano più colori della stessa palette.
	}
	else	//Schermo sub
	{
		oamClearSprite(&oamSub, ufo->getNOam());
		u16* ufogfxmain = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color);
		dmaCopy(ufoTiles, ufogfxmain, ufoTilesLen);
		oamSet(&oamMain, ufo->getNOam(), newx, newy, 0, 0, SpriteSize_32x32, SpriteColorFormat_256Color, ufogfxmain + (512 * ufo->getCurrentFrame()), 0, false, false, false, false, false); //Versione 256 colori
		//oamSet(&oamMain, ufo->getNOam(), newx, newy, 0, 0, SpriteSize_32x32, SpriteColorFormat_256Color, ufogfxmain + (256 * ufo->getCurrentFrame()), 0, false, false, false, false, false); //Versione 16 colori
		ufo->setOam(1);
		ufo->setGfx(ufogfxmain);
		//ufo->getSprite()->palette = palettenum;
	}
}

void checkMoveScreen(Ufo *ufo)
{
	/* 
	Questa funzione controlla ad ogni frame se sia necessario spostare l'ufo passato come parametro su un altro schermo.
	*/
	
	if((isOffscreen(*(ufo->getSprite())) == 1) && ufo->getOam() == 1)
	{
		debugMSG("Uscito dal main, cambio schermo:", true);
		changeUfoScreen(ufo);
	}
	else
	{
		if((isOffscreen(*(ufo->getSprite())) == 2) && ufo->getOam() == 2)
		{
			debugMSG("Uscito dal sub, cambio schermo:", true);
			changeUfoScreen(ufo);
		}
	}
}

void getLissajousXY(int *x, int *y, int t, int xoffset, int yoffset)
{
	/* 
	Questa funzione usa un timer, t, in modo da impostare i valori di x e y di un oggetto secondo una curva di lissajous.
	
	Dettagli sulle curve di lissajous sono stati presi da queste pagine:
	
	https://it.wikipedia.org/wiki/Figura_di_Lissajous
	
	Generatore di formule di lissajous, usato per sceglierne una: https://academo.org/demos/lissajous-curves/
	
	Alla formula tradizionale, è stato aggiunto un valore di phase-shift, il quale imposta la posizione di partenza dell'oggetto all'interno della curva all'istante t = 0.
	
	NB: i valori sono stati scelti dopo molti test in modo da ottenere una curva abbastanza larga ed alta da attraversare tutto lo schermo senza incappare in bug con la funzione isOffscreen.
	Fare attenzione prima di aumentare i valori, e farlo in piccoli incrementi se si vuole usarlo in combinazione con movimento tra gli schermi.
	
	*/
	
	int alpha = 5;
	int beta = 4;
	*x = xoffset + (0.03 * sinLerp((alpha * t) + 1));
	*y = yoffset + (0.05 * sinLerp(beta * t));
}

void animateUFO2(int *timer, int t, Ufo *ufo, OamState &oam)
{
	//Animazione dell'ufo usando la sua classe.
	
	//debugMSG("Frame prima", ufo->getCurrentFrame());
	//Non uso i valori della spriteentry, altrimenti verrebbero passati per valore, il che crea problemi.
	int newx = ufo->getSprite()->x;
	int newy = ufo->getSprite()->y;
	
	getLissajousXY(&newx, &newy, t, 120, 160);	//Ottieni nuova posizione
	if(ufo->getOam() == 1)
		{
			newy = newy - 192; //Offset, necessario in quanto i valori di x e y generati dalla curva di lissajous vanno oltre le dimensioni di 1 schermo, e portano a bug. Riducendo così, si ottiene una curva naturale. 
		}
	oamSetXY(&oam, ufo->getNOam(), newx, newy);	//Imposta la nuova posizione
	if(*timer == 14)		//Ogni 14 tick, 1 frame di animazione. Questa sezione dovrà essere modificata quando si userà un timer hardware, o si cambia sprite.
	{
		int newframe = (ufo->getCurrentFrame() + 1) % ufo->getNFrames(); //Procedi al prossimo frame di animazione
		ufo->setCurrentFrame(newframe);	//Impostalo come frame corrente
		//Qui viene calcolato il prossimo frame di animazione con questa formula: indirizzo memoria video del primo frame + (dimensioni sprite (32*32) / 2) * numero del frame (necessario contare da 1))
		oamSet(&oam, ufo->getNOam(), newx, newy, 0, 0, SpriteSize_32x32, SpriteColorFormat_256Color, ufo->getGfx() + (512 * ufo->getCurrentFrame()), 0, false, false, false, false, false); //Versione 256 colori
		//oamSet(&oam, ufo->getNOam(), newx, newy, 0, 0, SpriteSize_32x32, SpriteColorFormat_16Color, ufo->getGfx() + (256 * ufo->getCurrentFrame()), 0, false, false, false, false, false); //Versione 16 colori
		*timer = 0;	//Resetta il timer per l'animazione
		//debugPosition(oam, 0, "UFO");
	}
	//debugMSG("Frame dopo", ufo->getCurrentFrame());
}

int main() 
{
	powerOn(POWER_ALL_2D); // Accende il motore grafico 2D.	
	lcdMainOnBottom(); // Piazza lo schermo main nello schermo fisico inferiore.
	initVideo();

	//Inizializza il motore per sprite su schermi main e sub, indichiamo offset in memoria degli sprite e che non vogliamo memoria estesa.

	oamInit(&oamMain, SpriteMapping_1D_128, false);
	oamInit(&oamSub, SpriteMapping_1D_128, false);

	vector<int> backgrounds = initBackgrounds();	//Carica sfondi, contenuti in questo vettore.
	
	int *rainbowcolors = getRainbowGradientAsArray();	//Serve per l'effetto arcobaleno
	
	
	//Crea sfondo sullo schermo superiore
	int subbg1 = bgInitSub(2, BgType_Bmp16, BgSize_B16_256x256, 0,0);
	u16* videoMemorySub = bgGetGfxPtr(subbg1);
	dmaCopy(Sfondosub1Bitmap, videoMemorySub, Sfondosub1BitmapLen);
		
	//EFFETTO PARALLASSE SFONDO MAIN
	
	float add1 = 20.0;
	float add2 = 26.5;
	s32 nadd1 = s32(add1 * 32);
	s32 nadd2 = s32(add2 * 32);
	s32 nbg1x = 0 << 8;
	s32 nbg2x = 0 << 8;
	bgWrapOn(backgrounds[0]);
	bgWrapOn(backgrounds[1]);
	
	//Creazione sprite UFO
	dmaCopy(ufoPal, &SPRITE_PALETTE_SUB[0], ufoPalLen); //COPIA IN MEMORIA LA PALETTE DELL'UFO ALLA MEMORIA PER SCHERMO SUB DI PALETTE PER SPRITE.
	u16* ufogfx = oamAllocateGfx(&oamSub, SpriteSize_32x32, SpriteColorFormat_256Color);
	dmaCopy(ufoTiles, ufogfx, ufoTilesLen);
	
	
	//COPIA IN MEMORIA IL FONT PER LO SCROLLER

	dmaCopy(TestVert3Pal, &SPRITE_PALETTE[0], TestVert3PalLen);
	u16* lettergfx = oamAllocateGfx(&oamMain, SpriteSize_16x16, SpriteColorFormat_256Color);
	dmaCopy(TestVert3Tiles, lettergfx, TestVert3TilesLen);
	
	//INIZIALIZZATO TUTTO, PARTE IL TIMER PER EFFETTI
	
	s16 timerCounter = 0; //TIMER: +- 16 secondi per arrivare a 100, quindi 6,25 == 1 secondo.
	s16 angleCounter = 0; //Serve per l'effetto sinusoide
	
	//Scrivo i messaggi dello scroller
	string message1 = "Contraddizione fra due leggi corrette";
	string message2 = "Un omaggio alla demoscene Amiga";
	string message3 = "Invito a superare i nostri limiti";
	string message4 = "Intro creata da Francesco Maria Puca";
	string message5 = "Codice sorgente disponibile su Github";
	string message6 = "Ringraziamenti speciali";
	string message7 = "Sviluppatori libreria libnds";
	string message8 = "Dave Murphy Michael Noland Jason Rogers";
	string message9 = "Rafael vujk Alexei Karpenko";
	string message10 = "Sviluppatore grit";
	string message11 = "Jasper Vijn";
	string message12 = "Mantenitori progetto devkitpro";
	//string message1 = "Ma che bel gufo spenzola da quei travi"; //TEST TUTTE LE LETTERE	
	int message1length = message1.length();
	int message2length = message2.length();
	int message3length = message3.length();
	int message4length = message4.length();
	int message5length = message5.length();
	int message6length = message6.length();
	int message7length = message7.length();
	int message8length = message8.length();
	int message9length = message9.length();
	int message10length = message10.length();
	int message11length = message11.length();
	int message12length = message12.length();

	vector<int> messageOAMS1;
		
	int rainbowtimer = 0; //Serve a coordinare la velocità di scroll dell'arcobaleno. Inversamente proporzionale allo spessore.
	int lissajoustimer = 0;	//Serve a generare la curva di lissajous per l'ufo.
	int ufotimer = 0;	//Usato per il timer dell'animazione dell'ufo.
	
	Ufo ufo (2, 0, 4, ufogfx);	//Lo dichiaro in anticipo, altrimenti non lo trova nel while.

	//CICLO PRINCIPALE
	while(1)
	{
		//GENERAZIONE ARCOBALENO
		rainbowtimer += 1;
		showRainbowEffect(&rainbowtimer, 82, 9, 3, 256, videoMemorySub, rainbowcolors, 16, 48464);
		
		
		//CALCOLO NUOVA POSIZIONE DEGLI SFONDI
		
		nbg1x = nbg1x - nadd1;
		nbg2x = nbg2x - nadd2;
		
		timerCounter +=1; //AUMENTO TIMER

		debugMSG("Timer:", timerCounter);
		
		//DEBUG UFO, SALTA A POCO PRIMA LA COMPARSA
		/*if(timerCounter == 1)
		{
			timerCounter = 1740;
		}*/
		
		//Codice per l'entrata dello sfondo superiore: lo sposto di nascosto su, poi lo rendo visibile e lo sposto alla sua posizione originale.
		if(timerCounter == 1)
		{
			bgHide(subbg1);
		}
		if(timerCounter <= 200)
		{
			if(timerCounter == 101)
			{	
				bgShow(subbg1);
			}
			if(timerCounter <= 100)
			{
				bgScroll(subbg1, 0, 2); //Sposta fisicamente lo sfondo, Scrollf invece applica un' "accelerazione" allo sfondo nella sua posizione originale.
			}
			else
			{
				bgScroll(subbg1, 0, -2);
			}
		}
		/*Documentazione animazione messaggi: 
		
		Due timer, angleCounter e timerCounter. Timercounter fa da timer per gli eventi, anglecounter viene resettato ogni volta che 
		stampo le lettere di un nuovo messaggio ed aumenta solo mentre lo sto facendo scrollare. è quello che poi viene passato alla sinusoide.*/
		if(timerCounter == 180) //Timer rudimentale, ricordare l'uguale perchè altrimenti lo ripete.
		{
			messageOAMS1 = printSpacedCharMessageForSineWave(oamMain, lettergfx, 0, 256, 0, 12, message1);  //x = 256 pare essere una colonna invisibile fuori dallo schermo. Qui si trovano i vari sprite fuori schermo 
		}
		if(timerCounter == 900)
		{
			angleCounter = 0; //Resetta timer sinusoide
			messageOAMS1 = printSpacedCharMessageForSineWave(oamMain, lettergfx, 0, 256, 0, 12, message2);
		}
		if(timerCounter == 1500)
		{
			angleCounter = 0;
			messageOAMS1 = printSpacedCharMessageForSineWave(oamMain, lettergfx, 0, 256, 0, 12, message3);
		}
		if(timerCounter == 2100)
		{
			angleCounter = 0;
			messageOAMS1 = printSpacedCharMessageForSineWave(oamMain, lettergfx, 0, 256, 0, 12, message4);
		}
		if(timerCounter == 2750)
		{
			angleCounter = 0;
			messageOAMS1 = printSpacedCharMessageForSineWave(oamMain, lettergfx, 0, 256, 0, 12, message5);
		}
		if(timerCounter == 3450)
		{
			angleCounter = 0;
			messageOAMS1 = printSpacedCharMessageForSineWave(oamMain, lettergfx, 0, 256, 0, 12, message6);
		}
		if(timerCounter == 3950)
		{
			angleCounter = 0;
			messageOAMS1 = printSpacedCharMessageForSineWave(oamMain, lettergfx, 0, 256, 0, 12, message7);
		}
		if(timerCounter == 4550)
		{
			angleCounter = 0;
			messageOAMS1 = printSpacedCharMessageForSineWave(oamMain, lettergfx, 0, 256, 0, 12, message8);
		}
		if(timerCounter == 5250)
		{
			angleCounter = 0;
			messageOAMS1 = printSpacedCharMessageForSineWave(oamMain, lettergfx, 0, 256, 0, 12, message9);
		}
		if(timerCounter == 5800)
		{
			angleCounter = 0;
			messageOAMS1 = printSpacedCharMessageForSineWave(oamMain, lettergfx, 0, 256, 0, 12, message10);
		}
		if(timerCounter == 6300)
		{
			angleCounter = 0;
			messageOAMS1 = printSpacedCharMessageForSineWave(oamMain, lettergfx, 0, 256, 0, 12, message11);
		}
		if(timerCounter == 6700)
		{
			angleCounter = 0;
			messageOAMS1 = printSpacedCharMessageForSineWave(oamMain, lettergfx, 0, 256, 0, 12, message12);
		}
		if(timerCounter >= 180 && timerCounter < 900)
		{
			angleCounter+=1; //Aumenta l'angolo della sinusoide finchè sto animando
				for(int i = 0; i < message1length; i++)				//TUTTI ASSIEME
					{
						onlyAnimateCharForSineWave(oamMain, i, angleCounter);
						checkSingleChar(oamMain, i, 0, 12);
					}
		}
		if(timerCounter >= 900 && timerCounter < 1500)
		{
			angleCounter+=1;
				for(int i = 0; i < message2length; i++)				
					{
						onlyAnimateCharForSineWave(oamMain, i, angleCounter);
						checkSingleChar(oamMain, i, 0, 12);
					}
		}
		if(timerCounter >= 1500 && timerCounter < 2100)
		{
			angleCounter+=1;
				for(int i = 0; i < message3length; i++)				
					{
						onlyAnimateCharForSineWave(oamMain, i, angleCounter);
						checkSingleChar(oamMain, i, 0, 12);
					}
		}
		if(timerCounter >= 2100 && timerCounter < 2750)
		{
			angleCounter+=1;
				for(int i = 0; i < message4length; i++)				
					{
						onlyAnimateCharForSineWave(oamMain, i, angleCounter);
						checkSingleChar(oamMain, i, 0, 12);
					}
		}
		if(timerCounter >= 2750 && timerCounter < 3450)
		{
			angleCounter+=1;
				for(int i = 0; i < message5length; i++)				
					{
						onlyAnimateCharForSineWave(oamMain, i, angleCounter);
						checkSingleChar(oamMain, i, 0, 12);
					}
		}
		if(timerCounter >= 3450 && timerCounter < 3950)
		{
			angleCounter+=1;
				for(int i = 0; i < message6length; i++)				
					{
						onlyAnimateCharForSineWave(oamMain, i, angleCounter);
						checkSingleChar(oamMain, i, 0, 12);
					}
		}
		if(timerCounter >= 3950 && timerCounter < 4550)
		{
			angleCounter+=1;
				for(int i = 0; i < message7length; i++)				
					{
						onlyAnimateCharForSineWave(oamMain, i, angleCounter);
						checkSingleChar(oamMain, i, 0, 12);
					}
		}
		if(timerCounter >= 4550 && timerCounter < 5250)
		{
			angleCounter+=1;
				for(int i = 0; i < message8length; i++)				
					{
						onlyAnimateCharForSineWave(oamMain, i, angleCounter);
						checkSingleChar(oamMain, i, 0, 12);
					}
		}
		if(timerCounter >= 5250 && timerCounter < 5800)
		{
			angleCounter+=1;
				for(int i = 0; i < message9length; i++)				
					{
						onlyAnimateCharForSineWave(oamMain, i, angleCounter);
						checkSingleChar(oamMain, i, 0, 12);
					}
		}
		if(timerCounter >= 5800 && timerCounter < 6300)
		{
			angleCounter+=1;
				for(int i = 0; i < message10length; i++)				
					{
						onlyAnimateCharForSineWave(oamMain, i, angleCounter);
						checkSingleChar(oamMain, i, 0, 12);
					}
		}
		if(timerCounter >= 6300 && timerCounter < 6700)
		{
			angleCounter+=1;
				for(int i = 0; i < message11length; i++)				
					{
						onlyAnimateCharForSineWave(oamMain, i, angleCounter);
						checkSingleChar(oamMain, i, 0, 12);
					}
		}
		if(timerCounter >= 6700 && timerCounter < 7300)
		{
			angleCounter+=1;
				for(int i = 0; i < message12length; i++)				
					{
						onlyAnimateCharForSineWave(oamMain, i, angleCounter);
						checkSingleChar(oamMain, i, 0, 12);
					}
		}
		if(timerCounter == 7350)	//Entra in scena l'ufo
		{
			//Copia la palette dell'ufo in memoria palette dei due schermi per evitare di farlo ogni volta e rallentare
			dmaCopy(ufoPal, &SPRITE_PALETTE_SUB[0], ufoPalLen);
			dmaCopy(ufoPal, &SPRITE_PALETTE[0], ufoPalLen);

			oamSet(&oamSub, 0, -40, 0, 0, 0, SpriteSize_32x32, SpriteColorFormat_256Color, ufogfx, 0, false, false, false, false, false); //Piazza lo sprite dell'ufo, 256 colori
		}
		if(timerCounter >= 7350 && timerCounter <= 7385)	//Fai entrare l'ufo in posizione
		{
			int ufox = ufo.getSprite()->x;
			int ufoy = ufo.getSprite()->y;
			if(ufox  != 120 && ufoy != 160)
			{
				oamSetXY(&oamSub, 0, ufox + 4, ufoy + 4);
			}				
		}
		if(timerCounter >= 7385 && timerCounter < 8300)	//Anima l'ufo
		{
			lissajoustimer += 25;	//Questa addizione controlla la velocità con cui l'ufo si muove lungo la curva. Maggiori valori, più va veloce. Minori valori, più va lento.
			ufotimer += 1;
			checkMoveScreen(&ufo);

			//debugMSG("DEBUG: Stampo valore dell'oam passato all'ufo: ", ufo.getOam());
			//debugMSG("DEBUG: Stampo valore offscreen nel main: ", isOffscreen(*(ufo.getSprite())));
			
			//A seconda di dove si trova l'ufo, animalo su quello schermo.
			if((&ufo)->getOam() == 1)
			{
				animateUFO2(&ufotimer, lissajoustimer, &ufo, oamMain);
			}
			else
			{
				animateUFO2(&ufotimer, lissajoustimer, &ufo, oamSub);
			}
		}

		if(timerCounter == 8300)	//Smetti di animare l'ufo se va oltre questo tempo ed è uscito fuorischermo dal sub. Tarato sulla funzione di lissajous presente, se si modifica va cambiato.
		{
			oamClearSprite(&oamMain, ufo.getNOam());
		}

		//Effetto zoom
		if(timerCounter > 8300 && timerCounter <= 8450)
		{
			float x = (8450 - timerCounter);
			bgSetScale(backgrounds[0], x, x);
			bgSetScale(backgrounds[1], x, x);
		}

		swiWaitForVBlank();	//Aspetta che finisca di scannerizzare lo schermo
		
		//Aggiorna memoria sprite su main e sub
		oamUpdate(&oamSub);
		oamUpdate(&oamMain);

		//Effetto scroll per gli sfondi del main
		if(timerCounter < 8300)
		{
			bgSetScrollf(backgrounds[0], nbg2x, 0 << 8);
			bgSetScrollf(backgrounds[1], nbg1x, 0 << 8);
		}

		bgUpdate();	//Aggiorna gli sfondi
	}
	return 0;
}