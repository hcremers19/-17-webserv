<?php
if ($_SERVER["REQUEST_METHOD"] === "POST") {
    if (isset($_FILES["fileToUpload"])) {
        $targetDirectory = "uploads/"; // Répertoire de destination pour enregistrer les fichiers
        $targetFile = $targetDirectory . basename($_FILES["fileToUpload"]["name"]);
        $uploadOk = 1;
        $fileType = strtolower(pathinfo($targetFile, PATHINFO_EXTENSION));

        // Vérifier si le fichier est réellement un fichier
        if (!is_uploaded_file($_FILES["fileToUpload"]["tmp_name"])) {
            echo "Erreur : Aucun fichier n'a été sélectionné.";
            $uploadOk = 0;
        }

        // Vérifier la taille du fichier (ici, limitée à 5 Mo)
        if ($_FILES["fileToUpload"]["size"] > 5 * 1024 * 1024) {
            echo "Erreur : Le fichier est trop volumineux. La taille maximale autorisée est de 5 Mo.";
            $uploadOk = 0;
        }

        // Autoriser uniquement certains types de fichiers (ici, limité à des images)
        $allowedFileTypes = array("jpg", "jpeg", "png", "gif");
        if (!in_array($fileType, $allowedFileTypes)) {
            echo "Erreur : Seuls les fichiers JPG, JPEG, PNG et GIF sont autorisés.";
            $uploadOk = 0;
        }

        if ($uploadOk == 0) {
            echo "Erreur : Le fichier n'a pas été téléchargé.";
        } else {
            if (move_uploaded_file($_FILES["fileToUpload"]["tmp_name"], $targetFile)) {
                echo "Le fichier " . basename($_FILES["fileToUpload"]["name"]) . " a été téléchargé avec succès.";
            } else {
                echo "Erreur : Une erreur s'est produite lors du téléchargement du fichier.";
            }
        }
    } else {
        echo "Erreur : Aucun fichier n'a été sélectionné.";
    }
}
?>
