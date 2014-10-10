Úloha PEK: Permutace číselných koleček
============

## Vstupní data:

- n = délka rovnostranného trojúhelníka, n>=5
- q = přirozené číslo, n^2>q
- X0 = počáteční konfigurace zkonstruovaná zpětným provedením q náhodných tahu z cílové konfigurace. Platí q>=d(X0). 

## Pravidla a cíl hry:

Herní deska má tvar rovnostranného trojúhelníka o délce strany n, kde v i-tem řádku je i políček, ležících na průsečících úseček, rovnoběžných se stranami trojúhelníka. V těchto políčkách jsou podle určité permutace rozmístěna kolečka s čísly 1,…,M-1, kde M=n(n+1)/2. Jedno políčko zůstává volné, viz příklad na obrázku vlevo.

Tomuto rozmístění koleček budeme říkat počáteční konfigurace X0. Jeden tah je přesun kolečka na sousední volné políčko ve směru některé úsečky. Cílem hry je použitím minimálního počtu tahů převést počáteční konfiguraci X0 do cílové konfigurace C, ve které jsou kolečka seřazena vzestupně po řádcích tak, že políčko na horním vrcholu trojúhelníkové desky je volné, viz obrázek vpravo. Úloha má vždy řešení.

## Definice:

Je-li X konfigurace rozmístění všech koleček na herní desce, pak

- t(X) je počet doposud provedených tahů, kterými jsme převedli počáteční konfiguraci X0 do konfigurace X.
- d(X) je spodní mez počtu tahů, kterými se lze dostat z konfigurace X do cílové konfigurace C. Tato spodní mez je rovna součtu vzdáleností koleček od jejich cílových políček. Vzdálenost 2 políček v této síti se počítá takto: Jsou-li obě políčka na úsečce rovnoběžné se stranou trojúhelníka, pak je vzdálenost rovna jejich lineární vzdálenosti po této úsečce. V opačném případě tvoří políčka vrcholy kosodélníka a vzdálenost se rovná součtu délek jeho dvou stran. Spodní mez počtu tahů nejlepšího možného řešení je tedy d(X0).

## Generování počátečního stavu:

X0 vygenerujeme nejprve q náhodně provedenými zpětnými tahy z cílové konfigurace C.

## Výstup algoritmu:

Výpis nejkratší posloupnosti tahů vedoucí z počáteční konfigurace do cílové konfigurace.

## Sekvenční algoritmus:

Sekvenční algoritmus je typu BB-DFS s neomezenou hloubkou stromu konfiguraci. Přípustný stav je cesta z počáteční do cílové konfigurace C. Cena, která se minimalizuje, je počet tahů takové cesty.

Horní mez počtu tahů je q.

Dolní mez je d(X0).

## Paralelní algoritmus:

Paralelní algoritmus je typu L-PBB-DFS-D.

