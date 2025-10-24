/*
 * Binární vyhledávací strom — iterativní varianta
 *
 * S využitím datových typů ze souboru btree.h, zásobníku ze souboru stack.h
 * a připravených koster funkcí implementujte binární vyhledávací
 * strom bez použití rekurze.
 */

#include "../btree.h"
#include "stack.h"
#include <stdio.h>
#include <stdlib.h>

/*
 * Inicializace stromu.
 *
 * Uživatel musí zajistit, že inicializace se nebude opakovaně volat nad
 * inicializovaným stromem. V opačném případě může dojít k úniku paměti (memory
 * leak). Protože neinicializovaný ukazatel má nedefinovanou hodnotu, není
 * možné toto detekovat ve funkci.
 */
void bst_init(bst_node_t **tree)
{
    *tree = NULL;
}

/*
 * Vyhledání uzlu v stromu.
 *
 * V případě úspěchu vrátí funkce hodnotu true a do proměnné value zapíše
 * ukazatel na obsah daného uzlu. V opačném případě funkce vrátí hodnotu false a proměnná
 * value zůstává nezměněná.
 *
 * Funkci implementujte iterativně bez použité vlastních pomocných funkcí.
 */
bool bst_search(bst_node_t *tree, char key, bst_node_content_t **value)
{
    bst_node_t *current = tree;

    while (current != NULL)
    {
        if (current->key == key)
        {
            *value = &current->content;
            return true;
        }

        if (key < current->key)
        {
            current = current->left;
        }
        else
        {
            current = current->right;
        }
    }

    return false;
}

/*
 * Vložení uzlu do stromu.
 *
 * Pokud uzel se zadaným klíče už ve stromu existuje, nahraďte jeho hodnotu.
 * Jinak vložte nový listový uzel. V případě úspěchu funkce tuto skutečnost
 * indikuje návratovou hodnotou true, v opačném případě (např. při selhání
 * alokace) vrací funkce false.
 *
 * Výsledný strom musí splňovat podmínku vyhledávacího stromu — levý podstrom
 * uzlu obsahuje jenom menší klíče, pravý větší.
 *
 * Funkci implementujte iterativně bez použití vlastních pomocných funkcí.
 */
bool bst_insert(bst_node_t **tree, char key, bst_node_content_t value)
{
    bst_node_t **current = tree;

    while (true)
    {
        if (*current == NULL)
        {
            bst_node_t *new_node = malloc(sizeof(bst_node_t));
            if (new_node == NULL)
                return false;

            new_node->key = key;
            new_node->content = value;
            new_node->left = NULL;
            new_node->right = NULL;

            *current = new_node;
            return true;
        }

        if (key == (*current)->key)
        {
            (*current)->content = value;
            return true;
        }

        if (key < (*current)->key)
        {
            current = &(*current)->left;
        }
        else
        {
            current = &(*current)->right;
        }
    }

    return false;
}

/*
 * Pomocná funkce která nahradí uzel nejpravějším potomkem.
 *
 * Klíč a hodnota uzlu target budou nahrazené klíčem a hodnotou nejpravějšího
 * uzlu podstromu tree. Nejpravější potomek bude odstraněný. Funkce korektně
 * uvolní všechny alokované zdroje odstraněného uzlu.
 *
 * Funkce předpokládá, že hodnota tree není NULL.
 *
 * Tato pomocná funkce bude využita při implementaci funkce bst_delete.
 *
 * Funkci implementujte iterativně bez použití vlastních pomocných funkcí.
 */
void bst_replace_by_rightmost(bst_node_t *target, bst_node_t **tree)
{
    while ((*tree)->right != NULL)
    {
        tree = &(*tree)->right;
    }
    target->key = (*tree)->key;
    target->content = (*tree)->content;
    bst_node_t *old_node = *tree;
    *tree = (*tree)->left;
    free(old_node);
}

/*
 * Odstranění uzlu ze stromu.
 *
 * Pokud uzel se zadaným klíčem neexistuje, funkce nic nedělá.
 * Pokud má odstraněný uzel jeden podstrom, zdědí ho rodič odstraněného uzlu.
 * Pokud má odstraněný uzel oba podstromy, je nahrazený nejpravějším uzlem
 * levého podstromu. Nejpravější uzel nemusí být listem.
 *
 * Funkce korektně uvolní všechny alokované zdroje odstraněného uzlu.
 *
 * Funkci implementujte iterativně pomocí bst_replace_by_rightmost a bez
 * použití vlastních pomocných funkcí.
 */
void bst_delete(bst_node_t **tree, char key)
{
    while (*tree != NULL)
    {
        if (key < (*tree)->key)
        {
            tree = &(*tree)->left;
        }
        else if (key > (*tree)->key)
        {
            tree = &(*tree)->right;
        }
        else
        {
            if ((*tree)->left == NULL)
            {
                bst_node_t *old_node = *tree;
                *tree = (*tree)->right;
                free(old_node);
            }
            else if ((*tree)->right == NULL)
            {
                bst_node_t *old_node = *tree;
                *tree = (*tree)->left;
                free(old_node);
            }
            else
            {
                bst_replace_by_rightmost(*tree, &(*tree)->left);
            }
            return;
        }
    }
}

/*
 * Zrušení celého stromu.
 *
 * Po zrušení se celý strom bude nacházet ve stejném stavu jako po
 * inicializaci. Funkce korektně uvolní všechny alokované zdroje rušených
 * uzlů.
 *
 * Funkci implementujte iterativně s pomocí zásobníku a bez použití
 * vlastních pomocných funkcí.
 */
void bst_dispose(bst_node_t **tree)
{
    stack_bst_t next;
    stack_bst_init(&next);

    if (*tree != NULL)
    {
        stack_bst_push(&next, *tree);
    }

    while (!stack_bst_empty(&next))
    {
        bst_node_t *current = stack_bst_pop(&next);

        if (current->left != NULL)
        {
            stack_bst_push(&next, current->left);
        }
        if (current->right != NULL)
        {
            stack_bst_push(&next, current->right);
        }

        free(current);
    }

    *tree = NULL;
}

/*
 * Pomocná funkce pro iterativní preorder.
 *
 * Prochází po levé větvi k nejlevějšímu uzlu podstromu.
 * Nad zpracovanými uzly zavolá bst_add_node_to_items a uloží je do zásobníku uzlů.
 *
 * Funkci implementujte iterativně s pomocí zásobníku a bez použití
 * vlastních pomocných funkcí.
 */
void bst_leftmost_preorder(bst_node_t *tree, stack_bst_t *next, bst_items_t *items)
{
    bst_node_t *current = tree;

    while (current != NULL)
    {
        bst_add_node_to_items(current, items);
        stack_bst_push(next, current);
        current = current->left;
    }
}

/*
 * Preorder průchod stromem.
 *
 * Pro aktuálně zpracovávaný uzel zavolejte funkci bst_add_node_to_items.
 *
 * Funkci implementujte iterativně pomocí funkce bst_leftmost_preorder a
 * zásobníku uzlů a bez použití vlastních pomocných funkcí.
 */
void bst_preorder(bst_node_t *tree, bst_items_t *items)
{
    stack_bst_t next;
    stack_bst_init(&next);

    bst_leftmost_preorder(tree, &next, items);

    while (!stack_bst_empty(&next))
    {
        bst_node_t *current = stack_bst_pop(&next);
        bst_leftmost_preorder(current->right, &next, items);
    }
}

/*
 * Pomocná funkce pro iterativní inorder.
 *
 * Prochází po levé větvi k nejlevějšímu uzlu podstromu a ukládá uzly do
 * zásobníku uzlů.
 *
 * Funkci implementujte iterativně s pomocí zásobníku a bez použití
 * vlastních pomocných funkcí.
 */
void bst_leftmost_inorder(bst_node_t *tree, stack_bst_t *next)
{
    bst_node_t *current = tree;

    while (current != NULL)
    {
        stack_bst_push(next, current);
        current = current->left;
    }
}

/*
 * Inorder průchod stromem.
 *
 * Pro aktuálně zpracovávaný uzel zavolejte funkci bst_add_node_to_items.
 *
 * Funkci implementujte iterativně pomocí funkce bst_leftmost_inorder a
 * zásobníku uzlů a bez použití vlastních pomocných funkcí.
 */
void bst_inorder(bst_node_t *tree, bst_items_t *items)
{
    stack_bst_t next;
    stack_bst_init(&next);

    bst_leftmost_inorder(tree, &next);

    while (!stack_bst_empty(&next))
    {
        bst_node_t *current = stack_bst_pop(&next);
        bst_add_node_to_items(current, items);
        bst_leftmost_inorder(current->right, &next);
    }
}

/*
 * Pomocná funkce pro iterativní postorder.
 *
 * Prochází po levé větvi k nejlevějšímu uzlu podstromu a ukládá uzly do
 * zásobníku uzlů. Do zásobníku bool hodnot ukládá informaci, že uzel
 * byl navštíven poprvé.
 *
 * Funkci implementujte iterativně pomocí zásobníku uzlů a bool hodnot a bez použití
 * vlastních pomocných funkcí.
 */
void bst_leftmost_postorder(bst_node_t *tree, stack_bst_t *next,
                            stack_bool_t *first_iter)
{
    bst_node_t *current = tree;

    while (current != NULL)
    {
        stack_bst_push(next, current);
        stack_bool_push(first_iter, true);
        current = current->left;
    }
}

/*
 * Postorder průchod stromem.
 *
 * Pro aktuálně zpracovávaný uzel zavolejte funkci bst_add_node_to_items.
 *
 * Funkci implementujte iterativně pomocí funkce bst_leftmost_postorder a
 * zásobníku uzlů a bool hodnot a bez použití vlastních pomocných funkcí.
 */
void bst_postorder(bst_node_t *tree, bst_items_t *items)
{
    stack_bst_t next;
    stack_bool_t first_iter;
    stack_bst_init(&next);
    stack_bool_init(&first_iter);

    bst_leftmost_postorder(tree, &next, &first_iter);

    while (!stack_bst_empty(&next))
    {
        bst_node_t *current = stack_bst_pop(&next);
        bool is_first = true;
        if (!stack_bool_empty(&first_iter))
        {
            is_first = stack_bool_pop(&first_iter);
        }
        else
        {
            break;
        }

        if (is_first)
        {
            stack_bst_push(&next, current);
            stack_bool_push(&first_iter, false);
            bst_leftmost_postorder(current->right, &next, &first_iter);
        }
        else
        {
            bst_add_node_to_items(current, items);
        }
    }
}
