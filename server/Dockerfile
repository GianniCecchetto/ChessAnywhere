# Étape 1 : Build avec Maven
FROM maven:3.9.8-eclipse-temurin-21 AS builder

# Copier le code source dans l’image
WORKDIR /app
COPY /target/server-1.0-SNAPSHOT.jar /app/server.jar

# Compiler le projet et créer le JAR (skip tests si besoin)
RUN ./mvnw clean package -DskipTests

# Étape 2 : Image finale pour exécution
FROM eclipse-temurin:21-jdk-alpine

# Dossier de travail
WORKDIR /app

# Copier le JAR construit depuis l’étape précédente
COPY --from=builder /app/target/*.jar app.jar

# Exposer le port 8080
EXPOSE 8080

# Commande de lancement
ENTRYPOINT ["java", "-jar", "server.jar"]
