# MeshViewer

Visualiseur de maillages 3D développé dans le cadre du cours de **Modélisation Géométrique** à l'ESIEE Paris. Le programme charge des fichiers `.obj`, les représente via une structure de données half-edge, et propose plusieurs algorithmes de traitement de maillages.

## Fonctionnalités implémentées

| Algorithme | Description |
|---|---|
| **Lecture de fichier OBJ** | Chargement et construction complète de la structure half-edge |
| **Calcul de normales** | Normales par face (produit vectoriel) et par sommet (moyenne des faces adjacentes) |
| **Silhouette** | Détection et rendu des arêtes de silhouette par rapport à la caméra |
| **Triangulation** | Ear-clipping avec test barycentrique — gère les faces convexes et concaves |
| **Tests half-edge** | Vérification de la cohérence des pointeurs twin, next, prev et source |
| **Surface de révolution** | Génération d'un maillage quad par rotation d'un profil autour de l'axe Y |
| **Simplification** | Réduction du maillage par collapse itératif de l'arête la plus courte (~50 % de sommets) |
| **Subdivision Catmull-Clark** | Subdivision de surfaces avec gestion des bords et des faces quelconques |

## Dépendances

- CMake ≥ 3.10
- C++17
- OpenGL
- GLEW (via Homebrew : `/opt/homebrew`)
- GLUT
- GLM

## Compilation

```bash
mkdir build && cd build
cmake ..
make
```

## Utilisation

```bash
cd build
./MeshViewer
```

Le programme charge automatiquement `c_gear.obj` au démarrage et affiche le maillage dans une fenêtre OpenGL. Toutes les fonctions sont accessibles via le **clic droit** :

| Menu | Actions disponibles |
|---|---|
| **Draw** | Affichage du maillage, wireframe, normales, silhouette, sommets, Catmull-Clark |
| **Mesh Operations** | Lisser, gonfler (inflate) |
| **Select** | Sélectionner l'arête / face / sommet le plus proche |
| **Face Operations** | Découper une arête ou une face |
| **Racine** | Ouvrir un fichier, triangulate, surface de révolution, simplifier, quitter |

### Navigation caméra

| Action | Contrôle |
|---|---|
| Rotation | Clic gauche + glisser |
| Zoom | Molette / clic droit + glisser |

## Fichiers maillages fournis

- `cube.obj`
- `apple.obj`
- `dolphin.obj`
- `hand.obj`

## Structure du code

```
myMesh.{h,cpp}       — Maillage et algorithmes principaux
myHalfedge.{h,cpp}   — Structure half-edge
myVertex.{h,cpp}     — Sommet et normale par sommet
myFace.{h,cpp}       — Face et normale par face
myPoint3D.{h,cpp}    — Point 3D
myVector3D.{h,cpp}   — Vecteur 3D
main.cpp             — Interface OpenGL/GLUT et tests
helperFunctions.h    — Utilitaires OpenGL (buffers, shaders, menus)
```
