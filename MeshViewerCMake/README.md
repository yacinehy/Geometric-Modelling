# MeshViewer — Modélisation Géométrique

Visualiseur de maillages 3D développé dans le cadre du cours de **Modélisation Géométrique** à l'ESIEE Paris.  
Le programme charge des fichiers `.obj`, les représente via une structure de données **half-edge**, et propose plusieurs algorithmes de traitement de maillages polygonaux.

---

## Fonctionnalités implémentées

### 1. Lecture de fichier OBJ et construction half-edge

Le parser lit les lignes `v` (sommets) et `f` (faces) d'un fichier `.obj`. Pour chaque face, un tableau de half-edges est instancié et les relations `next`/`prev` sont câblées en anneau. Les jumeaux (`twin`) sont établis à l'aide d'une `std::map<pair<int,int>, myHalfedge*>` qui associe chaque paire ordonnée de sommets à sa half-edge : si la paire inverse existe déjà dans la map, le jumelage est réalisé immédiatement.

### 2. Calcul de normales

- **Par face** : produit vectoriel de deux arêtes consécutives, normalisé.  
- **Par sommet** : moyenne des normales des faces incidentes (parcours du 1-voisinage via les relations `twin->next`), normalisée. Ce choix (moyenne simple, non pondérée) est rapide mais peut favoriser les petites faces sur un maillage non uniforme ; une pondération par l'aire améliorerait la qualité.

### 3. Détection de silhouette

Pour chaque arête intérieure (twin non nul), on teste le signe du produit scalaire de la direction caméra avec la normale des deux faces adjacentes. Si les signes sont opposés, l'arête est une silhouette. Le rendu est effectué directement en mode immédiat OpenGL (liste d'indices reconstruite à chaque frame).

### 4. Triangulation par Ear-Clipping

L'algorithme triangule une face quelconque (N sommets) :

1. Calcul de la normale du polygone par la méthode de Newell (robuste aux faces non planes).  
2. Recherche d'une **oreille** : sommet convexe dont le triangle formé avec ses deux voisins ne contient aucun autre sommet du polygone (test barycentrique).  
3. Détachement de l'oreille : création d'une nouvelle face triangulaire, une paire de half-edges internes, et mise à jour des relations `next`/`prev`/`twin`.  
4. Répétition jusqu'à ce qu'il ne reste que 3 sommets.

Un chemin de repli (fallback) garantit la terminaison même sur des polygones quasi-dégénérés.

### 5. Surface de révolution

Un profil 2D (liste de `myPoint3D`) est tourné en `steps` pas autour de l'axe Y. Les quads obtenus sont construits explicitement avec leurs 4 half-edges, et les jumeaux entre quads adjacents (horizontalement et verticalement) sont câblés par calcul d'index. La surface est ensuite normalisée pour tenir dans [-1, 1].

### 6. Simplification par collapse de l'arête la plus courte

L'algorithme réduit le nombre de sommets d'environ **50 %** par collapse itératif :

**Principe du collapse d'une arête (v_keep, v_remove) :**

```
Avant :                    Après :

   v_a                        v_a
  / | \                      /   \
v_keep-v_remove    →      v_mid
  \ | /                      \   /
   v_b                        v_b
```

À chaque itération :
1. Recherche de l'arête non-orientée la plus courte (distance euclidienne²).
2. Déplacement de `v_keep` au **milieu** de l'arête.
3. Reconnexion des twins externes : les voisins des deux triangles supprimés deviennent twins entre eux, court-circuitant les faces disparues.
4. Redirection de toutes les half-edges dont `source == v_remove` vers `v_keep`.
5. Correction des pointeurs `originof` des sommets affectés (`v_keep`, `v_a`, `v_b`).
6. Suppression des 6 half-edges, 2 faces et 1 sommet (marqués `nullptr`, compactés en fin de passe).

Des gardes protègent contre les configurations dégénérées (boucle, sandwich, `v_a == v_b`).

**Complexité** : O(V²) — acceptable pour les maillages du cours ; une file de priorité (heap) permettrait du O(V log V).

**Limitation connue** : le critère de coût est purement géométrique (longueur). Des approches plus avancées (Garland-Heckbert, QEM) préservent mieux les arêtes de caractère.

### 7. Tests de cohérence

Trois tests sont exécutés au chargement :

| Test | Ce qui est vérifié |
|---|---|
| `testHalfEdgesValidity` | `twin->twin == self`, `next->prev == self`, `prev->next == self`, source non nul |
| `testTriangulationValidity` | Toutes les faces ont exactement 3 arêtes après triangulation |
| `testNormalsValidity` | Toutes les normales (faces et sommets) sont unitaires (tolérance 1e-4) |

---

## Dépendances

- CMake ≥ 3.10, C++17
- OpenGL, GLEW, GLUT, GLM

Installation des dépendances sur macOS :

```bash
brew install glew glm freeglut
```

## Compilation

```bash
cd MeshViewerCMake
mkdir build && cd build
cmake ..
make
```

## Utilisation

```bash
# depuis le dossier contenant les fichiers .obj
cd MeshViewerCMake
./build/MeshViewer
```

Le programme charge `c_gear.obj` au démarrage. Toutes les fonctions sont accessibles via **clic droit**.

### Navigation caméra

| Action | Contrôle |
|---|---|
| Rotation | Clic gauche + glisser |
| Zoom | Molette |

---

## Structure du code

```
myMesh.{h,cpp}       — Maillage + algorithmes (triangulation, simplification, révolution…)
myHalfedge.{h,cpp}   — Structure half-edge
myVertex.{h,cpp}     — Sommet et normale par sommet
myFace.{h,cpp}       — Face et normale par face
myPoint3D.{h,cpp}    — Point 3D
myVector3D.{h,cpp}   — Vecteur 3D
main.cpp             — Boucle OpenGL/GLUT, menus, tests
helperFunctions.h    — Utilitaires OpenGL (VBO, shaders, rendu texte)
```

---

## Utilisation de l'IA

Un assistant IA (GitHub Copilot) a été utilisé de manière ponctuelle pour :

- **Déboguer** la gestion des pointeurs `originof` lors du collapse d'arête — après avoir écrit la logique principale, certains cas limites produisaient des accès invalides que l'IA a aidé à identifier.
- **Reformuler** des conditions de test dans `testHalfEdgesValidity` pour couvrir les deux sens (`next->prev` et `prev->next`).

L'ensemble de la conception algorithmique (structure du collapse, ear-clipping, surface de révolution) a été développé en autonomie, en s'appuyant sur les cours et les références bibliographiques du module.
