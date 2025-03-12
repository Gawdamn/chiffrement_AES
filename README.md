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
   ```
   git clone https://github.com/Gawdamn/chiffrement_AES.git
   cd chiffrement_AES
   ```
2. Assurez-vous que les dépendances suivantes sont installées :
   - Qt 5
   - OpenSSL
   - GCC (ou un autre compilateur C++)
3. Compilez le projet :
     ```
     ./build.sh
     ```
4. Il vous suffit maintenant de lancer l'exécutable "ChiffrAES" qui a été généré pour démarrer l'application!

---  


# **Guide d'utilisation de ChiffrAES**
## 1. Interface principale

L'interface de ChiffrAES est organisée en deux onglets principaux:

- **Interface principale**: Contient les boutons pour chiffrer et déchiffrer des fichiers.
- **Historique**: Affiche l'historique des opérations effectuées.

La barre de menu en haut donne accès aux options, à l'aide et aux informations sur l'application.

La barre d'état en bas de la fenêtre affiche des informations sur l'état actuel de l'application et une barre de progression pendant les opérations de chiffrement/déchiffrement.

## 2. Chiffrement d'un fichier

Pour chiffrer un fichier:

1. Cliquez sur le bouton **Chiffrer un fichier** dans l'onglet principal ou utilisez le raccourci clavier **Ctrl+C**.

2. Une boîte de dialogue s'ouvrira pour sélectionner le fichier à chiffrer. Naviguez jusqu'au fichier souhaité et cliquez sur **Ouvrir**.

3. Une fenêtre de saisie du mot de passe apparaîtra. Entrez un mot de passe sécurisé.
   - Un indicateur de force du mot de passe s'affiche sous forme de barre colorée:
     - Rouge: Mot de passe faible
     - Jaune: Mot de passe moyen
     - Vert: Mot de passe fort
   - Vous pouvez cocher la case "Afficher le mot de passe" pour voir ce que vous tapez.

4. Après avoir validé le mot de passe, une boîte de dialogue vous demandera de choisir l'emplacement et le nom du fichier chiffré.
   - Par défaut, l'extension ".aes" sera ajoutée au nom du fichier.

5. L'application commencera le chiffrement. Une barre de progression s'affichera dans la barre d'état.

6. Une fois le chiffrement terminé, un message de confirmation apparaîtra.
   - Si l'option "Supprimer le fichier original après chiffrement" est activée dans les paramètres, le fichier source sera automatiquement supprimé.

## 3. Déchiffrement d'un fichier

Pour déchiffrer un fichier:

1. Cliquez sur le bouton **Déchiffrer un fichier** dans l'onglet principal ou utilisez le raccourci clavier **Ctrl+D**.

2. Une boîte de dialogue s'ouvrira pour sélectionner le fichier à déchiffrer. Les fichiers chiffrés ont généralement l'extension ".aes".

3. Une fenêtre de saisie du mot de passe apparaîtra. Entrez le mot de passe utilisé lors du chiffrement.
   - Contrairement au chiffrement, l'évaluation de la force du mot de passe n'est pas affichée lors du déchiffrement.

4. Une fois le mot de passe validé, l'application vérifiera d'abord son exactitude avant de vous demander de choisir l'emplacement pour le fichier déchiffré.

5. L'application commencera le déchiffrement. Une barre de progression s'affichera dans la barre d'état.

6. Une fois le déchiffrement terminé, un message de confirmation apparaîtra.

**Note importante**: Si vous saisissez un mot de passe incorrect ou si le fichier est corrompu, un message d'erreur s'affichera et aucun fichier ne sera généré.

## 4. Options de configuration

Pour accéder aux options de configuration:

1. Cliquez sur le menu en haut à gauche de l'application.
2. Sélectionnez **Options**.

La boîte de dialogue des options permet de configurer:

- **Taille de clé AES**: Choisissez entre AES-128, AES-192 ou AES-256 bits. Une clé plus grande offre une sécurité théorique supérieure mais peut être légèrement plus lente.

- **Mode sombre**: Active ou désactive l'interface en mode sombre (couleurs plus foncées pour réduire la fatigue oculaire).

- **Supprimer le fichier original après chiffrement**: Si activée, cette option supprime automatiquement le fichier source après un chiffrement réussi.

- **Activer l'historique des opérations**: Si activée, l'application conservera un historique des opérations de chiffrement et déchiffrement effectuées.

Cliquez sur **OK** pour enregistrer vos modifications ou sur **Annuler** pour les ignorer.

## 5. Utilisation de l'historique

L'onglet **Historique** affiche un tableau contenant les informations sur toutes les opérations de chiffrement et déchiffrement effectuées:

- **Date**: Date et heure de l'opération
- **Opération**: Type d'opération (Chiffrement ou Déchiffrement)
- **Fichier source**: Chemin du fichier original
- **Fichier destination**: Chemin du fichier résultant
- **Statut**: Résultat de l'opération (Succès ou Échec)

Pour effacer l'historique:

1. Cliquez sur le bouton **Supprimer l'historique** situé sous le tableau.
2. Confirmez votre choix dans la boîte de dialogue qui apparaît.

**Note**: Si vous avez désactivé l'option "Activer l'historique des opérations" dans les paramètres, les nouvelles opérations ne seront pas enregistrées, mais l'historique existant restera visible jusqu'à ce qu'il soit effacé.

---  
## FAQ

### J'ai oublié mon mot de passe, puis-je récupérer mon fichier?
Non. Pour des raisons de sécurité, ChiffrAES n'offre aucun moyen de récupérer un fichier si vous avez oublié le mot de passe. Il n'existe pas de "porte dérobée" ou de système de récupération de mot de passe.

### L'application affiche "Le mot de passe est incorrect ou les données sont corrompues"
Ce message peut apparaître pour plusieurs raisons:
- Vous avez entré un mot de passe incorrect
- Le fichier chiffré a été modifié ou endommagé après sa création
- Le fichier n'est pas un fichier chiffré avec ChiffrAES

Vérifiez votre mot de passe et assurez-vous que le fichier n'a pas été modifié depuis son chiffrement.

### Le chiffrement/déchiffrement est-il sécurisé?
ChiffrAES utilise l'algorithme AES en mode GCM, reconnu comme standard de sécurité. La sécurité dépend également de la force de votre mot de passe - utilisez toujours des mots de passe longs et complexes.

### Puis-je chiffrer des dossiers entiers?
Dans la version actuelle, ChiffrAES ne prend en charge que le chiffrement de fichiers individuels. Pour chiffrer un dossier, vous devez d'abord le compresser (par exemple en .zip) puis chiffrer l'archive résultante.

### Combien de temps prend le chiffrement/déchiffrement?
Le temps de traitement dépend de la taille du fichier et de la puissance de votre ordinateur. Les petits fichiers (quelques Mo) sont généralement traités en quelques secondes, tandis que les fichiers volumineux (plusieurs Go) peuvent prendre plusieurs minutes.

### Puis-je utiliser ChiffrAES sur Windows ou macOS?
ChiffrAES a été développé spécifiquement pour Linux. Bien que Qt soit multiplateforme, des modifications seraient nécessaires pour assurer la compatibilité avec d'autres systèmes d'exploitation.

### L'application se bloque pendant une opération
Si l'application semble bloquée pendant une opération sur un fichier volumineux, soyez patient. L'interface reste réactive grâce au traitement asynchrone, et la barre de progression devrait vous indiquer l'avancement. Si aucun progrès n'est visible après plusieurs minutes, vous pouvez redémarrer l'application.

---

## **Licence**
GPL-3.0