# **Système de verrouillage de fichiers basé sur le chiffrement AES**

## **Description du projet**
Ce projet est une application de bureau destinée aux systèmes Linux, permettant aux utilisateurs de protéger leurs fichiers en les chiffrant et en les déchiffrant grâce à l'algorithme AES (Advanced Encryption Standard). Développée en C++ avec le framework Qt, cette application offre une interface intuitive pour gérer la confidentialité et l'intégrité des fichiers sensibles.

---

## **Fonctionnalités principales**
- **Chiffrement des fichiers** : Sélectionnez un ou plusieurs fichiers et protégez-les avec une clé dérivée d'un mot de passe.
- **Déchiffrement des fichiers** : Déverrouillez vos fichiers chiffrés en saisissant le mot de passe correct.
- **Gestion sécurisée des mots de passe** : Dérivation des clés via PBKDF2 avec sel unique et un nombre élevé d'itérations.
- **Interface utilisateur conviviale** : Navigation simple et intuitive pour sélectionner les fichiers et effectuer les opérations.
- **Support universel** : Compatible avec tous les types de fichiers (texte, images, vidéos, archives, etc.).

---

## **Architecture logicielle**
L'application suit une architecture **Modèle-Vue-Contrôleur (MVC)** pour séparer les responsabilités :
1. **Modèle (Model)** : Implémente les algorithmes de chiffrement/déchiffrement, dérive les clés, et gère les opérations sur les fichiers.
2. **Vue (View)** : Interface utilisateur créée avec Qt Designer, affichant les options et recueillant les entrées de l'utilisateur.
3. **Contrôleur (Controller)** : Coordonne les interactions entre la Vue et le Modèle, traite les actions utilisateur et met à jour l'interface.

---

## **Objectifs du projet**
- Fournir une solution simple et efficace pour sécuriser les fichiers sensibles.
- Garantir une expérience utilisateur fluide sur les systèmes Linux, en particulier Manjaro.
- Tester les performances sur des fichiers de différentes tailles pour assurer la robustesse.

---

## **Exigences techniques**
- **Système d'exploitation** : Linux (Manjaro recommandé).
- **Langage de programmation** : C++.
- **Framework UI** : Qt (version 5.15 ou supérieure).
- **Bibliothèque de chiffrement** : OpenSSL.
- **IDE** : Qt Creator.

---

## **Instructions pour l'installation et l'exécution**
1. Clonez le dépôt Git :
   ```bash
   git clone https://github.com/Gawdamn/chiffrement_AES.git
   cd chiffrement_AES
   ```
2. Assurez-vous que les dépendances suivantes sont installées :
   - Qt 5
   - OpenSSL
   - GCC (ou un autre compilateur C++)
3. Si vous utilisez le shell ZSH, lancez le script "build.zsh"
   Sinon, lancez les commandes suivantes dans l'ordre:
   ```bash
   qmake
   make
   make clean
   ```
4. Il vous suffit maintenant de cliquer sur l'exécutable "ChiffrAES" qui a été généré pour lancer l'application!

---

## **Tests et validation**
- Tests fonctionnels sur différents types et tailles de fichiers.
- Vérification de la gestion des erreurs : mot de passe incorrect, fichiers corrompus, permissions insuffisantes.
- Validation des performances et de la sécurité.


## **Licence**
GPL-3.0