# Rattrapage 25 Mars 2022
# Traitement d'images & filtres

___

## Description du programme à paralléliser

Le but de cet examen est d'optimiser puis de paralléliser, en mémoire partagée puis en mémoire distribuée, un programme permettant d'appliquer divers filtres pour transformer des images.

Ces filtres sont des convolutions d'ordre un ou deux (c'est à dire prenant en compte les voisins (ordre un) ainsi que les voisins des voisins (ordre 2)). On applique cette convolution à une image pour obtenir une nouvelle image.

Par exemple, le filtre gaussien est un filtre permettant de détecter les changements brusques de couleurs ou de contraste qui se calcule comme suit :

    r_n(i,j) = 4.r_o(i,j) - r_o(i-1,j) - r_o(i+1,j) - r_o(i,j-1) - r_o(i,j+1)

où `r_n(i,j)` est la valeur de la composante rouge du pixel de la nouvelle image sur la ligne i et la colonne j et `r_o(i,j)` la valeur de la composante rouge du pixel situé à la ligne i et la colonne j de l'image sur laquelle on applique le filtre.

Pour traiter les pixels se trouvant au bord de l'image, on rajoute une rangée de pixels "fantômes" sur chaque côté de l'image. Les valeurs des composantes rouges, vertes et bleues de ces pixels sont les  copies des valeurs des pixels voisins appartenant à l'image. Si on utilise un filtre d'ordre deux, on dégénère le filtre à l'ordre un au bord de l'image afin de ne pas à avoir à rajouter une seconde couche de pixels fantômes.

De plus, afin de faciliter les calculs, l'image est d'abord "normalisée" et les composantes représentées par des réels compris entre 0 et 1.

Pour exécuter le programme, on lance :

    ./filtres_images.exe choix <image>

où `choix` est un entier valant 1, 2 ou 3. `image` est le nom d'une image à traiter qui vaut par défaut `data/lena_gray.png`. Cette image, relativement petite, vous permettra de "déboguer" votre programme au cours de l'examen.

En prenant choix valant :

    choix = 1 : On applique un filtre laplacien d'ordre 1 : laplacien + inversion couleur
    choix = 2 : On applique un filtre laplacien d'ordre 2 : flou gaussien + laplacien + inversion couleur
    choix = 3 : On applique un filtre renforçant la nettteté de l'image (ordre 1)
    choix = 4 : On applique un flou gaussien
    choix = 5 : On inverse les couleurs de l'image

Exemples d'utilisation :

_Appliquer une laplacien d'ordre deux sur `pexels-arthouse-studio-4311512.png`_

    ./filtres_images.exe 2 data/pexels-arthouse-studio-4311512.png

_Appliquer le filtre renforçant la netteté sur l'image par défaut (lena)_

    ./filtres_images.exe 3

Dans tous les cas, l'image obtenue est sauvegardée en sortie dans `image_filtree.png`.

Plusieurs images sont fournies dans le répertoire `data`. Pour développer votre code, l'image `lena_gray.png` est de taille assez petite pour être traitée rapidement.

Si votre ordinateur est assez puissant, vous pouvez ensuite utiliser l'image `pexels-arthouse-studio-4311512.png` pour mesurer les performances de votre code.

Si votre ordinateur prend trop de temps pour cette image, préférez prendre l'image `pexels-ahmad-ramadan-131811.png` plus petite.

Notez qu'il est probable que la partie du code la plus lente sera la sauvegarde de l'image filtrée. Le temps affichée pour cette partie du code **n'est qu'informative** et il est inutile de chercher à paralléliser ou à optimiser cette partie du code (ainsi que le chargement de l'image d'ailleurs)

## Travail à effectuer

Il est important de conserver à chaque étape de l'examen les sources du programme précédent afin d'être correctement évalué !

### Optimisation du programme

Sans avoir à paralléliser, étudier le programme qui vous est fourni et optimiser le en cherchant à modifier l'ordre des boucles afin d'accélérer le traitement séquentiel des images.

Mesurez les performances obtenues et comparez les avec le programme d'origine

Expliquez votre démarche et analysez pourquoi votre version du code est plus rapide que la version donnée initialement.

**Indication** : Normalement, il est possible de diviser le temps des fonctions par un facteur supérieur à deux (jusqu'à dix sur certains ordinateurs).

### Parallélisation en mémoire partagée

Paralléliser les fonctions à l'aide d'OpenMP.

Mesurez les performances obtenues et l'accélération en fonction du nombre de threads pris pour chaque fonction.

Interprétez vos résultats et expliquez pourquoi vous avez ou non obtenue une accélération pour chaque fonction.

### Parallélisation en mémoire distribuée

L'idée est de découper l'image en plusieurs bandes horizontales et de rajouter une couche de pixels fantômes servant d'échange avec les processus voisins.

Paralléliser le code avec MPI puis mesurer l'accélération obtenue sur votre ordinateur selon le nombre de processus.

Pourquoi l'accélération est moindre que ce qu'on est en droit d'attendre ?

Si vous aviez à votre disposition plusieurs machines reliées par un réseau dédié, quel pourrait être à votre avis le comportement de l'accélération obtenue ?

___
