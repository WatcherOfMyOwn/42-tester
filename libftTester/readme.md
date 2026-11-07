# libft-tester

Un tester plus ou moins développé pour le projet **libft** de l'école 42 (promo 2026). gère les (`malloc`) et l'analyse des comportements indéfinis (*Undefined Behavior*).

!! Conçus à l'aide d'IA, donc peux avoir des comportements indéfinis.

## Structure des Tests

Le testeur est divisé en plusieurs grandes sections :

1. **Partie 1 : Fonctions de la Libc** (`isalpha`, `strlen`, `memset`, `memcpy`, `memmove`, `memcmp`, `atoi`, etc.) :
   - Comparaison stricte avec la libc sur de larges plages de valeurs (ex: `-1` à `255` pour le `ctype`).
2. **Partie 2 : Fonctions additionnelles** (`substr`, `strjoin`, `strtrim`, `split`, `itoa`, `strmapi`, etc.).
3. **Partie Bonus : Listes chaînées** (`ft_lstnew`, `ft_lstadd_front`, `ft_lstmap`, `ft_lstclear`, etc.).
4. **Échecs de Malloc (`MFAIL`)** : Test itératif de chaque allocation pour s'assurer qu'un `malloc` raté renvoie bien `NULL` sans fuite.
5. **Cas limites valides (`Edge Cases`)** : Vérification que les listes vides, les tailles extrêmes (`SIZE_MAX`) ou les descripteurs invalides (`fd = -1`) ne font pas crasher le programme.
6. **Comportements Indéfinis (`Undefined Behaviour`)** : Analyse comparative avec la libc face à des comportements non définis par la norme (ex: passages de pointeurs `NULL`) ou tests unitaire de comportements indéfinis.

## Utilisation

Place ton dossier de test à la racine de ton projet `libft` (ou configure le chemin `LIBFT_DIR` dans le Makefile si besoin).

**Recommandé:** Utiliser `make asan` et `make vg` pour testé. Nettoyer avec `make vclean`. Utiliser `make run` si problème rencontrer avec les 2 premiers (mais testera moins profondément le code et necessitera completion par d'autre test).

### Commandes disponibles :

* **Lancer les tests standards** :
  ```bash
  make run
  ```
  * **Ce qu'il fait :** Le strict minimum et n'est pas reellement suffisant. Je l'ai mis plus par completion et au cas ou ASan ne fonctionne pas correctement.

Pour les tests plus approfondis de malloc, il va falloir utiliser valgrind ou ASan

* **Lancer les tests avec Valgrind (recherches de fuites)** :
  ```bash
  make vg
  ```
  * **Ce qu'il fait :** Lance le testeur sous *Valgrind* pour traquer les fuites (`malloc` sans `free`) et les variables non initialisées.
  * **valgrind.log:** Le testeur gère automatiquement les rapports de processus (`fork`) et nettoie les logs propres. S'il y a un vrai problème, il conservera un fichier **`valgrind.log`** à la racine pour que tu puisses analyser l'erreur en détail.

* **Lancer les tests avec AddressSanitizer & UBSan** :
  ```bash
  make asan
  ```
  * **Ce qu'il teste :** Les dépassements de mémoire (*buffer overflow*) et les accès invalides.
  * **Cas d'erreur :** Si ton code segfault ou dépasse un tableau, il devrait s'arreter net et te donner **la ligne exacte** du problème.

Je conseille personnellement d'utiliser asan puis vg pour être sur à la fin.

* **Nettoyer les log valgrind** :
  ```bash
  make vclean
  ```

## Légende des Rapports

* `GREEN` **OK** : Le comportement est parfaitement conforme à la libc ou aux attentes du sujet.
* `RED` **KO** : Écart détecté par rapport au comportement attendu (erreur critique).
* `YELLOW` / `ORANGE` **UB / CRASH** : Cas indéfinis (comparatif indicatif avec la libc, non comptabilisé comme un échec bloquant).

## Auteur

Développé par **Watcher of my Own ■■ (Hanajiime)** et mon ami claude.