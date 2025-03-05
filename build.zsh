#!/bin/zsh
# Script pour lancer automatiquement cmake, make et make clean

echo "Lancement de cmake..."
if ! cmake; then
    echo "Erreur lors de l'exécution de cmake"
    exit 1
fi

echo "Lancement de make..."
if ! make; then
    echo "Erreur lors de l'exécution de make"
    exit 1
fi

echo "Nettoyage avec make clean..."
if ! make clean; then
    echo "Erreur lors de l'exécution de make clean"
    exit 1
fi

clear
echo "Opérations terminées avec succès !\nVous pouvez désormais lancer l'application en cliquant sur l'excécutable nommé interface_AES"
