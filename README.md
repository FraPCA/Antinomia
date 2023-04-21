# Antinomia

Progetto personale di programmazione Nintendo DS che si ispira alle intro Demoscene degli anni 90.

Il codice sorgente è commentato, al fine di documentare gli effetti realizzati e di fornire un esempio realistico di programmazione su quest'architettura, utilizzando una versione moderna della libreria libnds.

# Istruzioni per la compilazione

Per compilare il programma, è necessario installare la toolchain devkitPro seguendo le istruzioni, assicurandosi di avere installato correttamente la libreria libnds.

Successivamente, eseguire il comando `make` da una finestra di terminale aperta nella cartella in cui si trova questo file.

Se il makefile non è stato modificato e l'installazione è andata a buon fine, verranno generati due file, `Antinomia.elf` ed `Antinomia.nds`.

Pacchetto di installazione devkitPro: --> https://devkitpro.org/wiki/Getting_Started

# Strumenti per la programmazione Nintendo DS

La realizzazione di questo progetto è interamente dovuta alla presenza degli strumenti e delle guide citate in questa sezione.

Non ho creato nessuno degli elementi in questa sezione, nè ne sono in alcun modo responsabile e/o direttamente affiliato.

Tutti i diritti sono riservati ai rispettivi autori, inclusi quelli di copyright.

La libreria principale usata nel codice è libnds, disponibile alla pagina github: --> https://github.com/devkitPro/libnds

Libnds fa parte della toolchain devkitPro, disponibile alla pagina github: --> https://github.com/devkitPro/  

Questi due strumenti sono indispensabili per incominciare a programmare applicazioni per Nintendo DS.

Per convertire le immagini in un formato utilizzabile, si utilizza grit oppure la sua controparte grafica, wingrit: --> https://www.coranac.com/projects/#grit

Una scheda tecnica dell'architettura Nintendo DS è presente nell'eccellente risorsa gbatek: --> https://problemkaputt.de/gbatek.htm

Questo sito permette di calcolare lo spazio disponibile, le modalità da utilizzare ed eventuali conflitti di memoria video che potrebbero capitare: --> https://mtheall.com/banks.html

La guida realizzata da Jaeden Amero sulla programmazione di software homebrew Nintendo DS con la libreria libnds è una risorsa fondamentale: --> https://www.patater.com/files/projects/manual/manual.html

# Licenza

Fare riferimento al file `COPYRIGHTS.md`.
