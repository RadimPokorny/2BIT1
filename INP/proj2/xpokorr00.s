; Autor reseni: Radim Pokorny xpokorr00
;
; Projekt 2 - INP 2025
; Souhlaskove modulovana samohlaskova sifra na architekture MIPS64
; DATA SEGMENT
                .data
msg:            .asciiz "radimpokorny"          ; sem doplnte vase "jmenoprijmeni" (radimpokorny)
cipher:         .space  31                      ; misto pro zapis zasifrovaneho textu
                                                ; zde si muzete nadefinovat vlastni promenne ci konstanty,
                                                ; napr. hodnoty posuvu pro jednotlive znaky sifrovacho klice
params_sys5:    .space  8                       ; misto pro ulozeni adresy pocatku
                                                ; retezce pro vypis pomoci syscall 5
                                                ; (viz nize "funkce" print_string)
; CODE SEGMENT
                .text
main:
                daddi   r1, r0, msg             ; r1 = adresa vstupniho retezce
                daddi   r2, r0, cipher          ; r2 = adresa vystupu
                daddi   r5, r0, 26              ; r5 = posledni souhlaska
loop_read:
                lbu     r3, 0(r1)               ; nacteni znaku
                beq     r3, r0, konec           ; jestli konec retezce, skoc na konec
                                                ; podminky pro samohlasky
                daddi   r6, r0, 97              ; 'a'
                beq     r3, r6, samohlaska_case
                daddi   r6, r0, 101             ; 'e'
                beq     r3, r6, samohlaska_case
                daddi   r6, r0, 105             ; 'i'
                beq     r3, r6, samohlaska_case
                daddi   r6, r0, 111             ; 'o'
                beq     r3, r6, samohlaska_case
                daddi   r6, r0, 117             ; 'u'
                beq     r3, r6, samohlaska_case
                daddi   r6, r0, 121             ; 'y'
                beq     r3, r6, samohlaska_case
; ---------- souhlaska ----------
souhlaska_case:
                sb      r3, 0(r2)               ; ulozeni souhlasky
                daddi   r6, r3, -97             ; pozice v abecede 0 az 25
                daddi   r6, r6, 1               ; pozice 1 az 26
                daddi   r5, r6, 0               ; uloz jako posledni souhlasku
                daddi   r1, r1, 1               ; posun na dalsi znak
                daddi   r2, r2, 1               ; posun na dalsi pozici ve vystupu
                j       loop_read               ; pokracuj ve zpracovani
; ---------- samohlaska ----------
samohlaska_case:
                daddi   r6, r3, -97             ; offset 0 az 25
                daddu   r6, r6, r5              ; pridani klice
mod_loop:
                daddi   r7, r6, -26             ; kontrola prekroceni 'z'
                slt     r8, r7, r0              ; jestli mensi nez 0
                bne     r8, r0, modulo_done     ; pokud ne, skoc na modulo_done
                daddi   r6, r6, -26             ; jinak odecti 26
                j       mod_loop                ; opakuj kontrolu
modulo_done:
                daddi   r6, r6, 97              ; ASCII znak
                sb      r6, 0(r2)               ; ulozeni zasifrovane samohlasky
                daddi   r1, r1, 1               ; posun na dalsi znak  
                daddi   r2, r2, 1               ; posun na dalsi pozici ve vystupu
                j       loop_read               ; pokracuj ve zpracovani   
konec:
                sb      r0, 0(r2)               ; konec retezce
                daddi   r4, r0, cipher          ; priprava pro vypis zasifrovaneho textu
                jal     print_string            ; vypis zasifrovaneho textu
                syscall 0
; ---------- tiskovy syscall ----------
print_string:                                   ; adresa retezce se ocekava v r4
                sw      r4, params_sys5(r0)
                daddi   r14, r0, params_sys5    ; adr pro syscall 5 musi do r14
                syscall 5                       ; systemova procedura - vypis retezce na terminal
                jr      r31                     ; return - r31 je urcen na return address