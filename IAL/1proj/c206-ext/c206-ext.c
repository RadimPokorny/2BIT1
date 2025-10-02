/*
 *  Předmět: Algoritmy (IAL) - FIT VUT v Brně
 *  Rozšíření pro příklad c206.c (Dvousměrně vázaný lineární seznam)
 *  Vytvořil: Daniel Dolejška, září 2024
 */

#include "c206-ext.h"

bool error_flag;
bool solved;

/**
 * Tato metoda simuluje příjem síťových paketů s určenou úrovní priority.
 * Přijaté pakety jsou zařazeny do odpovídajících front dle jejich priorit.
 * "Fronty" jsou v tomto cvičení reprezentovány dvousměrně vázanými seznamy
 * - ty totiž umožňují snazší úpravy pro již zařazené položky.
 *
 * Parametr `packetLists` je dvousměrně vázaný seznam a jako položky obsahuje
 * jednotlivé seznamy (fronty) paketů (`QosPacketListPtr`). Pokud fronta
 * s odpovídající prioritou neexistuje, tato funkce ji alokuje a inicializuje.
 * Za jejich korektní uvolnení odpovídá volající.
 *
 * V případě, že by po zařazení paketu do seznamu počet prvků v cílovém seznamu
 * překročil stanovený MAX_PACKET_COUNT, dojde nejdříve k promazání položek seznamu.
 * V takovémto případě bude každá druhá položka ze seznamu zahozena nehledě
 * na její vlastní prioritu ovšem v pořadí přijetí.
 *
 * @param packetLists Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param packet Ukazatel na strukturu přijatého paketu
 */
void receive_packet(DLList *packetLists, PacketPtr packet)
{
	// Získání prvního prvku
	DLL_First(packetLists);
	// Založení cílové fronty a její inicializace na NULL
	QosPacketListPtr targetList = NULL;
	// Cyklus, který projde všechny prvky seznamu, přičemž na konci se vždy načte další prvek, až žádný není, cyklus skončí
	while (DLL_IsActive(packetLists))
	{
		// Získání hodnoty prvku
		QosPacketListPtr packetList;
		DLL_GetValue(packetLists, (long *)&packetList);
		// Pokud se priority shodují, tak je cílová fronta nalezena a cyklus končí
		if (packetList->priority == packet->priority)
		{
			targetList = packetList;
			break;
		}
		// ... další prvek
		DLL_Next(packetLists);
	}

	// Pokud fronta neexistuje, je vytvořena a inicializována
	if (targetList == NULL)
	{
		targetList = malloc(sizeof(QosPacketList));
		targetList->priority = packet->priority;
		DLL_Init(targetList->list);
		DLL_InsertLast(packetLists, (long)targetList);
	}

	// Paket je předán do cílové fronty
	DLL_InsertLast(targetList->list, (long)packet);

	// Pokud došlo k přetečení...
	if (targetList->list->currentLength > MAX_PACKET_COUNT)
	{
		// Nastavíme první prvek cílového seznamu jako aktivní
		DLL_First(targetList->list);
		// Proměnná pro sledování parity prvků
		int index = 1;
		// Cyklus, kde se prochází každý prvek cílového seznamu
		while (DLL_IsActive(targetList->list))
		{
			// Pokud je packet sudý, bude odstraněn
			if (index % 2 == 0)
			{
				// Pokud je aktivní prvek prvním prvkem, smaže se první prvek
				if (targetList->list->activeElement == targetList->list->firstElement)
				{
					DLL_DeleteFirst(targetList->list);
				}
				// Pokud je aktivní prvek posledním prvkem, smaže se poslední prvek a cyklus skončí
				else if (targetList->list->activeElement == targetList->list->lastElement)
				{
					DLL_DeleteLast(targetList->list);
					break;
				}
				// Jinak se smaže aktivní prvek
				else
				{
					// Posun na předchozí prvek (nový aktivní prvek)
					DLL_Previous(targetList->list);
					// Smazání původního aktivního prvku
					DLL_DeleteAfter(targetList->list);
					// Posun na prvek po smazaném (nový aktivní prvek)
					DLL_Next(targetList->list);
				}
			}
			// Pokud packet není sudý, pokračuje se na další prvek
			else
			{
				DLL_Next(targetList->list);
			}
			// Inkrementace indexu
			index++;
		}
	}
}

/**
 * Tato metoda simuluje výběr síťových paketů k odeslání. Výběr respektuje
 * relativní priority paketů mezi sebou, kde pakety s nejvyšší prioritou
 * jsou vždy odeslány nejdříve. Odesílání dále respektuje pořadí, ve kterém
 * byly pakety přijaty metodou `receive_packet`.
 *
 * Odeslané pakety jsou ze zdrojového seznamu při odeslání odstraněny.
 *
 * Parametr `packetLists` obsahuje ukazatele na jednotlivé seznamy paketů (`QosPacketListPtr`).
 * Parametr `outputPacketList` obsahuje ukazatele na odeslané pakety (`PacketPtr`).
 *
 * @param packetLists Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param outputPacketList Ukazatel na seznam paketů k odeslání
 * @param maxPacketCount Maximální počet paketů k odeslání
 */
void send_packets(DLList *packetLists, DLList *outputPacketList, int maxPacketCount)
{
	// Inicializace counter pro poslaný packety
	int sent = 0;

	// Běž tak dlouho, dokud nejsi na maxPacketCount
	while (sent < maxPacketCount)
	{
		// Inicializace packetu s nejvyšší prioritou
		QosPacketListPtr topPriority = NULL;

		// Získání prvího packetu
		DLL_First(packetLists);
		// Tento cyklus projede celou frontu s tím, že na konci se vždy načte další prvek, až žadný není, cyklus skončí
		while (DLL_IsActive(packetLists))
		{
			// Získání packetu
			QosPacketListPtr packetList;
			DLL_GetValue(packetLists, (long *)&packetList);
			// Pokud není nejvyšší priorita nebo je priorita větší než aktuální nejvyšší...
			if (!topPriority || packetList->priority > topPriority->priority)
			{
				// ...a zároveň fronta není prázdná, tak je nejvyšší priorita přepsána
				if (packetList->list->currentLength > 0)
					topPriority = packetList;
			}
			// Další prvek
			DLL_Next(packetLists);
		}

		// Pokud není žádný prvek s prioritou, ukončí se odeslání
		if (!topPriority)
			break;

		// Vezme se první prvek z fronty s nejvyšší prioritou
		DLL_First(topPriority->list);
		// Pokud je fronta prázdná, ukončí se odeslání
		if (!DLL_IsActive(topPriority->list))
			break;

		// Inicializace output packetu a získání jeho hodnoty
		PacketPtr outputPacket;
		DLL_GetValue(topPriority->list, (long *)&outputPacket);

		// Packet se přidá do output fronty
		DLL_InsertLast(outputPacketList, (long)outputPacket);

		// Packet je zahozen z původní fronty
		DLL_DeleteFirst(topPriority->list);

		// Inkrementace počtu odeslaných packetů
		sent++;
	}
}
