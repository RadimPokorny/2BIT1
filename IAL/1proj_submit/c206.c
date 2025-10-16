/* ******************************* c206.c *********************************** */
/*  Předmět: Algoritmy (IAL) - FIT VUT v Brně                                 */
/*  Úkol: c206 - Dvousměrně vázaný lineární seznam                            */
/*  Návrh a referenční implementace: Bohuslav Křena, říjen 2001               */
/*  Vytvořil: Martin Tuček, říjen 2004                                        */
/*  Upravil: Kamil Jeřábek, září 2020                                         */
/*           Daniel Dolejška, září 2021                                       */
/*           Daniel Dolejška, září 2022                                       */
/* ************************************************************************** */
/*
** Implementujte abstraktní datový typ dvousměrně vázaný lineární seznam.
** Užitečným obsahem prvku seznamu je hodnota typu int. Seznam bude jako datová
** abstrakce reprezentován proměnnou typu DLList (DL znamená Doubly-Linked
** a slouží pro odlišení jmen konstant, typů a funkcí od jmen u jednosměrně
** vázaného lineárního seznamu). Definici konstant a typů naleznete
** v hlavičkovém souboru c206.h.
**
** Vaším úkolem je implementovat následující operace, které spolu s výše
** uvedenou datovou částí abstrakce tvoří abstraktní datový typ obousměrně
** vázaný lineární seznam:
**
**      DLL_Init ........... inicializace seznamu před prvním použitím,
**      DLL_Dispose ........ zrušení všech prvků seznamu,
**      DLL_InsertFirst .... vložení prvku na začátek seznamu,
**      DLL_InsertLast ..... vložení prvku na konec seznamu,
**      DLL_First .......... nastavení aktivity na první prvek,
**      DLL_Last ........... nastavení aktivity na poslední prvek,
**      DLL_GetFirst ....... vrací hodnotu prvního prvku,
**      DLL_GetLast ........ vrací hodnotu posledního prvku,
**      DLL_DeleteFirst .... zruší první prvek seznamu,
**      DLL_DeleteLast ..... zruší poslední prvek seznamu,
**      DLL_DeleteAfter .... ruší prvek za aktivním prvkem,
**      DLL_DeleteBefore ... ruší prvek před aktivním prvkem,
**      DLL_InsertAfter .... vloží nový prvek za aktivní prvek seznamu,
**      DLL_InsertBefore ... vloží nový prvek před aktivní prvek seznamu,
**      DLL_GetValue ....... vrací hodnotu aktivního prvku,
**      DLL_SetValue ....... přepíše obsah aktivního prvku novou hodnotou,
**      DLL_Previous ....... posune aktivitu na předchozí prvek seznamu,
**      DLL_Next ........... posune aktivitu na další prvek seznamu,
**      DLL_IsActive ....... zjišťuje aktivitu seznamu.
**
** Při implementaci jednotlivých funkcí nevolejte žádnou z funkcí
** implementovaných v rámci tohoto příkladu, není-li u funkce explicitně
** uvedeno něco jiného.
**
** Nemusíte ošetřovat situaci, kdy místo legálního ukazatele na seznam
** předá někdo jako parametr hodnotu NULL.
**
** Svou implementaci vhodně komentujte!
**
** Terminologická poznámka: Jazyk C nepoužívá pojem procedura.
** Proto zde používáme pojem funkce i pro operace, které by byly
** v algoritmickém jazyce Pascalovského typu implemenovány jako procedury
** (v jazyce C procedurám odpovídají funkce vracející typ void).
**
**/

#include "c206.h"

bool error_flag;
bool solved;

/**
 * Vytiskne upozornění na to, že došlo k chybě.
 * Tato funkce bude volána z některých dále implementovaných operací.
 */
void DLL_Error(void)
{
	printf("*ERROR* The program has performed an illegal operation.\n");
	error_flag = true;
}

/**
 * Provede inicializaci seznamu list před jeho prvním použitím (tzn. žádná
 * z následujících funkcí nebude volána nad neinicializovaným seznamem).
 * Tato inicializace se nikdy nebude provádět nad již inicializovaným seznamem,
 * a proto tuto možnost neošetřujte.
 * Vždy předpokládejte, že neinicializované proměnné mají nedefinovanou hodnotu.
 *
 * @param list Ukazatel na strukturu dvousměrně vázaného seznamu
 */
void DLL_Init(DLList *list)
{
	// Vše je nastaveno na nulu
	list->lastElement = NULL;
	list->activeElement = NULL;
	list->firstElement = NULL;
	list->currentLength = 0;
}

/**
 * Zruší všechny prvky seznamu list a uvede seznam do stavu, v jakém se nacházel
 * po inicializaci.
 * Rušené prvky seznamu budou korektně uvolněny voláním operace free.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void DLL_Dispose(DLList *list)
{
	// V Cyklu se pokaždé přistupuju k prvnímu prvku listu do té doby, než žádný nezbude
	DLLElementPtr elmPtr;
	while (list->firstElement != NULL)
	{
		// Předá se reference novému prvku
		elmPtr = list->firstElement;
		// Následující prvek se přesune na předchozí pozici (pozice -= 1)
		list->firstElement = list->firstElement->nextElement;
		// Uvolní se místo v paměti pro prvek
		free(elmPtr);
	}
	// Vše je vynulováno
	list->activeElement = NULL;
	list->lastElement = NULL;
	list->currentLength = 0;
}

/**
 * Vloží nový prvek na začátek seznamu list.
 * V případě, že není dostatek paměti pro nový prvek při operaci malloc,
 * volá funkci DLL_Error().
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param data Hodnota k vložení na začátek seznamu
 */
void DLL_InsertFirst(DLList *list, long data)
{
	// Alokuje se místo pro nový prvek
	DLLElementPtr newElm = (DLLElementPtr)malloc(sizeof(struct DLLElement));
	// Neprovedla se alokace, vypíše se error a konec funkce
	if (newElm == NULL)
	{
		DLL_Error();
		return;
	}
	// Nový prvek obdrží data
	newElm->data = data;
	// První prvek se přesune na další pozici (pozice += 1)
	newElm->nextElement = list->firstElement;
	// Vyunuluje se přechozí prvek nového prvku
	newElm->previousElement = NULL;
	// Pokud je první prvek lsitu NULL, tak se předchozímu prvku toho prvního přiřadí data nového prvku
	if (list->firstElement != NULL)
	{
		list->firstElement->previousElement = newElm;
	}
	// Jinak poslední prvek získá data z nového prvku
	else
	{
		list->lastElement = newElm;
	}
	// První prvek listu získá data nového prvku
	list->firstElement = newElm;
	// Inkrementuje se délka
	list->currentLength++;
}

/**
 * Vloží nový prvek na konec seznamu list (symetrická operace k DLL_InsertFirst).
 * V případě, že není dostatek paměti pro nový prvek při operaci malloc,
 * volá funkci DLL_Error().
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param data Hodnota k vložení na konec seznamu
 */
void DLL_InsertLast(DLList *list, long data)
{
	// Alokuje se místo pro nový prvek
	DLLElementPtr newElm = (DLLElementPtr)malloc(sizeof(struct DLLElement));
	// Alokace selže, konec
	if (newElm == NULL)
	{
		DLL_Error();
		return;
	}
	// nový prvek se naplní daty
	newElm->data = data;
	// Následující prvek není
	newElm->nextElement = NULL;
	// Předešlý prvek je ten, který je v listu poslední
	newElm->previousElement = list->lastElement;
	// Pokud je nyní poslední prvek NULL, tak se dalšímu prvku od posledního přidělí data od nového prvku
	if (list->lastElement != NULL)
	{
		list->lastElement->nextElement = newElm;
	}
	// Jinak je první prvek naplněn daty z nového prvku
	else
	{
		list->firstElement = newElm;
	}
	// Poslední prvek je nový prvek
	list->lastElement = newElm;
	// Inkrementuje se délka
	list->currentLength++;
}

/**
 * Nastaví první prvek seznamu list jako aktivní.
 * Funkci implementujte jako jediný příkaz, aniž byste testovali,
 * zda je seznam list prázdný.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void DLL_First(DLList *list)
{
	// Aktivní prvek je nyní ten první
	list->activeElement = list->firstElement;
}

/**
 * Nastaví poslední prvek seznamu list jako aktivní.
 * Funkci implementujte jako jediný příkaz, aniž byste testovali,
 * zda je seznam list prázdný.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void DLL_Last(DLList *list)
{
	// Aktivní prvek je nyní ten poslední
	list->activeElement = list->lastElement;
}

/**
 * Prostřednictvím parametru dataPtr vrátí hodnotu prvního prvku seznamu list.
 * Pokud je seznam list prázdný, volá funkci DLL_Error().
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param dataPtr Ukazatel na cílovou proměnnou
 */
void DLL_GetFirst(DLList *list, long *dataPtr)
{
	// Pokud je první prvek nula, tak není nic obdrženo, nýbrž jen vyvolán error
	if (list->firstElement == NULL)
	{
		DLL_Error();
		return;
	}
	// Pokud není splněna podmínka, tak se data z listu předají na pointer data
	*dataPtr = list->firstElement->data;
}

/**
 * Prostřednictvím parametru dataPtr vrátí hodnotu posledního prvku seznamu list.
 * Pokud je seznam list prázdný, volá funkci DLL_Error().
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param dataPtr Ukazatel na cílovou proměnnou
 */
void DLL_GetLast(DLList *list, long *dataPtr)
{
	// Pokud je poslední prvek NULL, tak je vypsán jen error
	if (list->lastElement == NULL)
	{
		DLL_Error();
		return;
	}
	// Na data pointer se dostanou data z posledního elementu listu
	*dataPtr = list->lastElement->data;
}

/**
 * Zruší první prvek seznamu list.
 * Pokud byl první prvek aktivní, aktivita se ztrácí.
 * Pokud byl seznam list prázdný, nic se neděje.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void DLL_DeleteFirst(DLList *list)
{
	// Pokud je první prvek NULL, tak se nic neprovede
	if (list->firstElement != NULL)
	{
		// První prvek na smazání se předá do jiné proměnné
		DLLElementPtr toDelete = list->firstElement;

		// Pokud je aktivní element ten první, tak bude NULL
		if (list->activeElement == list->firstElement)
		{
			list->activeElement = NULL;
		}

		// Prvnímu prvku se přidělí následující hodnota od toho prvku, který bude smazán
		list->firstElement = toDelete->nextElement;

		//Pokud je první prvek NULL, tak tak ten následující je taky NULL
		if (list->firstElement != NULL)
		{
			list->firstElement->previousElement = NULL;
		}
		// V posledním případě je první prvek ten poslední, takže se smaže posledni
		else
		{
			list->lastElement = NULL;
		}
		// Uvolním místo v paměti, které bylo přiděleno proměnné toDelete
		free(toDelete);
		// Dekrementace délky 
		list->currentLength--;
	}
}

/**
 * Zruší poslední prvek seznamu list.
 * Pokud byl poslední prvek aktivní, aktivita seznamu se ztrácí.
 * Pokud byl seznam list prázdný, nic se neděje.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void DLL_DeleteLast(DLList *list)
{
	// Pokud je last element NULL, tak se nic nestane
	if (list->lastElement != NULL)
	{
		// Předá se poslední prvek do nové proměnné
		DLLElementPtr toDelete = list->lastElement;

		// Pokud je aktivní prvek ten poslední, tak se aktivní vynuluje
		if (list->activeElement == list->lastElement)
		{
			list->activeElement = NULL;
		}

		// Do posledního prvku dám hnodotu z předchozího prvku, který je uchován v prvku, který bude smazán
		list->lastElement = toDelete->previousElement;

		// Pokud poslední prvek je NULL, tak následující prvek toho posledního bude NULL
		if (list->lastElement != NULL)
		{
			list->lastElement->nextElement = NULL;
		}
		// Poslední možnost je, že první prvek je ten poslední, a tak se vynuluje ten první
		else
		{
			list->firstElement = NULL;
		}
		// Uvolní se místo v paměti, které bylo alokováno pro prvek k smazání 
		free(toDelete);
		// Dekrementuje se délka
		list->currentLength--;
	}
}

/**
 * Zruší prvek seznamu list za aktivním prvkem.
 * Pokud je seznam list neaktivní nebo pokud je aktivní prvek
 * posledním prvkem seznamu, nic se neděje.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void DLL_DeleteAfter(DLList *list)
{
	// Operace se bude provádět pouze, pokud existuje aktivní prvek a jeho následující prvek
	if (list->activeElement != NULL && list->activeElement->nextElement != NULL)
	{
		// Inicializace prvku, který, bude muset být smazán
		DLLElementPtr toDelete = list->activeElement->nextElement;

		// do následujícího prvku toho aktivního předáme data dalšího z toho, který má být smazán
		list->activeElement->nextElement = toDelete->nextElement;

		// Pokud je další prvek toho, který chcí smazat, je NULL...
		if (toDelete->nextElement != NULL)
		{
			// Předchozí prvek toho následujícího k smazání se předá aktivní element od list
			toDelete->nextElement->previousElement = list->activeElement;
		}
		else
		{
			/// Pokud byl mazaný prvek ten poslední, tak se poslednímu předá ten aktivní prvek
			list->lastElement = list->activeElement;
		}

		// Uvolní se místo v paměti, které bylo použito pro proměnnou toDelete
		free(toDelete);
		// Dekrementace délky
		list->currentLength--;
	}
}

/**
 * Zruší prvek před aktivním prvkem seznamu list .
 * Pokud je seznam list neaktivní nebo pokud je aktivní prvek
 * prvním prvkem seznamu, nic se neděje.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void DLL_DeleteBefore(DLList *list)
{
	// Pokud aktivní prvek není NULL a předešlý prvek toho aktivního není NULL, tak ..
	if (list->activeElement != NULL && list->activeElement->previousElement != NULL)
	{
		// Inicializace prvku k smazání
		DLLElementPtr toDelete = list->activeElement->previousElement;

		// Předchozímu prvku toho aktivního se přidělí předchozí prvek toho, který má být smazán
		list->activeElement->previousElement = toDelete->previousElement;

		// Pkud předchozí prvek toho, který bude smazán, není NULL, tak se dalšímu toho přecdchozímu předá aktivní element listu
		if (toDelete->previousElement != NULL)
		{
			toDelete->previousElement->nextElement = list->activeElement;
		}
		// Poslední možnost, že ten první bude aktivní prvek
		else
		{
			list->firstElement = list->activeElement;
		}

		// Uvolnění paměti rezervovanou pro proměnnou
		free(toDelete);
		// Dekrementace délky
		list->currentLength--;
	}
}

/**
 * Vloží prvek za aktivní prvek seznamu list.
 * Pokud nebyl seznam list aktivní, nic se neděje.
 * V případě, že není dostatek paměti pro nový prvek při operaci malloc,
 * volá funkci DLL_Error().
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param data Hodnota k vložení do seznamu za právě aktivní prvek
 */
void DLL_InsertAfter(DLList *list, long data)
{
	// Pokud aktivní prvek není NULL, tak se provede...
	if (list->activeElement != NULL)
	{
		// Alokace paměti pro nový prvek
		DLLElementPtr newElm = (DLLElementPtr)malloc(sizeof(struct DLLElement));
		// Pokud alokace selže, vyvolá se error
		if (newElm == NULL)
		{
			DLL_Error();
			return;
		}
		// Do nového prvku se předají data
		newElm->data = data;
		// předchozí prvek nového elementu bude ten aktivní
		newElm->previousElement = list->activeElement;
		// Další prvek bude ten následující aktivního prvku
		newElm->nextElement = list->activeElement->nextElement;

		// Následující prvek toho aktivního nyní bude ten nový prvek, pro který bylo alokováno místo
		list->activeElement->nextElement = newElm;

		// Pokud je další prvek NULL, tak přechozí prvek toho dalšího bude nový prvek
		if (newElm->nextElement != NULL)
		{
			newElm->nextElement->previousElement = newElm;
		}
		// V posledním případě se poslednímu prvku předá nový prvek
		else
		{
			list->lastElement = newElm;
		}
		// Inkrementace délky 
		list->currentLength++;
	}
}

/**
 * Vloží prvek před aktivní prvek seznamu list.
 * Pokud nebyl seznam list aktivní, nic se neděje.
 * V případě, že není dostatek paměti pro nový prvek při operaci malloc,
 * volá funkci DLL_Error().
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param data Hodnota k vložení do seznamu před právě aktivní prvek
 */
void DLL_InsertBefore(DLList *list, long data)
{
	// Pokud není active element null...
	if (list->activeElement != NULL)
	{
		// Alokuje se místo v paměti pro nový prvek
		DLLElementPtr newElm = (DLLElementPtr)malloc(sizeof(struct DLLElement));
		// Když selže alokace, tak se vrací error
		if (newElm == NULL)
		{
			DLL_Error();
			return;
		}
		/* 
		  Pokud není splněna podmínka, tak předáme novému prvku data, active element, 
		  který je pro něj next element a previous element od active elementu, který je pro něj active element
		*/
		newElm->data = data;
		newElm->nextElement = list->activeElement;
		newElm->previousElement = list->activeElement->previousElement;

		// Do listu previous elementu od active se předají data od proměnné "newElm"
		list->activeElement->previousElement = newElm;

		// Pokud je previous element od newElm NULL, tak se přidá od previousu next element data od newElmu
		if (newElm->previousElement != NULL)
		{
			newElm->previousElement->nextElement = newElm;
		}
		// Jinak se data nového prvku přiřadí prvnímu prvku listu
		else
		{
			list->firstElement = newElm;
		}
		// Inkrementace délky listu
		list->currentLength++;
	}
}

/**
 * Prostřednictvím parametru dataPtr vrátí hodnotu aktivního prvku seznamu list.
 * Pokud seznam list není aktivní, volá funkci DLL_Error ().
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param dataPtr Ukazatel na cílovou proměnnou
 */
void DLL_GetValue(DLList *list, long *dataPtr)
{
	// Pokud je aktivní element NULL, vrací se error
	if (list->activeElement == NULL)
	{
		DLL_Error();
		return;
	}
	// Pokud není splněna předchozí podmínka, tak pomocí activeElementu předáme data proměnné data pointeru
	*dataPtr = list->activeElement->data;
}

/**
 * Přepíše obsah aktivního prvku seznamu list.
 * Pokud seznam list není aktivní, nedělá nic.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param data Nová hodnota právě aktivního prvku
 */
void DLL_SetValue(DLList *list, long data)
{
	// Když je aktivní prvek NULL, funkce skončí
	if (list->activeElement == NULL)
	{
		return;
	}
	// Když není splněna předchozí podmínka, tak jsou předána data activeElementu
	list->activeElement->data = data;
}

/**
 * Posune aktivitu na následující prvek seznamu list.
 * Není-li seznam aktivní, nedělá nic.
 * Všimněte si, že při aktivitě na posledním prvku se seznam stane neaktivním.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void DLL_Next(DLList *list)
{
	// Pokud není aktivní prvek NULL, tak pomocí activeElement se získá nextElement
	if (list->activeElement != NULL)
	{
		list->activeElement = list->activeElement->nextElement;
	}
}

/**
 * Posune aktivitu na předchozí prvek seznamu list.
 * Není-li seznam aktivní, nedělá nic.
 * Všimněte si, že při aktivitě na prvním prvku se seznam stane neaktivním.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void DLL_Previous(DLList *list)
{
	// Pokud aktivní prvek není nula, tak je díky activeElement získán previousElement
	if (list->activeElement != NULL)
	{
		list->activeElement = list->activeElement->previousElement;
	}
}

/**
 * Je-li seznam list aktivní, vrací nenulovou hodnotu, jinak vrací 0.
 * Funkci je vhodné implementovat jedním příkazem return.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 *
 * @returns Nenulovou hodnotu v případě aktivity prvku seznamu, jinak nulu
 */
bool DLL_IsActive(DLList *list)
{
	// Pokud aktivní prvek není nula, tak je aktivní
	if (list->activeElement != NULL)
	{
		return true;
	}
	// Provede se pouze, pokud podmínka není splněna, je vráceno false
	return false;
}

/* Konec c206.c */
