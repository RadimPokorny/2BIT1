/*
 * Použití binárních vyhledávacích stromů.
 *
 * S využitím Vámi implementovaného binárního vyhledávacího stromu (soubory ../iter/btree.c a ../rec/btree.c)
 * implementujte triviální funkci letter_count. Všimněte si, že výstupní strom může být značně degradovaný
 * (až na úroveň lineárního seznamu). Jako typ hodnoty v uzlu stromu využijte 'INTEGER'.
 *
 */

#include "../btree.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * Vypočítání frekvence výskytů znaků ve vstupním řetězci.
 *
 * Funkce inicilializuje strom a následně zjistí počet výskytů znaků a-z (case insensitive), znaku
 * mezery ' ', a ostatních znaků (ve stromu reprezentováno znakem podtržítka '_'). Výstup je
 * ukládán průběžně do stromu (klíč vždy lowercase). V případě úspěchu funkce tuto skutečnost
 * indikuje návratovou hodnotou true, v opačném případě (např. při selhání
 * operace insert) vrací funkce false.
 *
 * Například pro vstupní řetězec: "abBccc_ 123 *" bude strom po běhu funkce obsahovat:
 *
 * key | value
 * 'a'     1
 * 'b'     2
 * 'c'     3
 * ' '     2
 * '_'     5
 *
 * Pro implementaci si můžete v tomto souboru nadefinovat vlastní pomocné funkce.
 */
bool letter_count(bst_node_t **tree, char *input)
{
    bst_init(tree);

    for (char *p = input; *p != '\0'; p++)
    {
        char symbol = *p;

        if (symbol >= 'A' && symbol <= 'Z')
        {
            symbol = symbol - 'A' + 'a';
        }
        else if (symbol < 'a' || symbol > 'z')
        {
            if (symbol == ' ')
            {
                symbol = ' ';
            }
            else
            {
                symbol = '_';
            }
        }

        bst_node_content_t *found_content;
        if (bst_search(*tree, symbol, &found_content))
        {
            (*(int *)(found_content->value))++;
        }
        else
        {
            int *val = malloc(sizeof(int));
            if (!val)
                return false;
            *val = 1;
            bst_node_content_t content;
            content.value = val;
            content.type = INTEGER;
            if (!bst_insert(tree, symbol, content))
            {
                free(val);
                return false;
            }
        }
    }
    return true;
}