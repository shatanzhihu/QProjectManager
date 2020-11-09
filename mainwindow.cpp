#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

    ui->setupUi(this);

    current_project = nullptr;

    // imposto il titolo
    setWindowTitle("Project manager - nessun progetto");
    // mostro una scritta sulla console
    showConsoleInfo("Caricamento completato correttamente");
    // aggiorno la status bar
    ui->statusbar->showMessage("Pronto per l'uso");
    // aggiungo l'item
    ui->objectsView->addItem("Nessun progetto creato");
    // creo il layout di anchor

    // carico i temi
    int res = loadThemes();
    qInfo() << res;
    // controlllo
    if(dark_style_sheet != ""){
        setStyleSheet(dark_style_sheet);
    }
    else{
        close();
    }
    // carico la palette logica
    loadLogicPalette();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionNuovo_triggered()
{
    // mostro il form di creazione del nuovo progetto
    NewProjectWin *newWin = new NewProjectWin();
    newWin->setStyleSheet(dark_style_sheet);
    int res = newWin->exec();
    // controllo il risultato del dialogo
    if(res == QDialog::Accepted){
          current_project = newWin->getProject();
          // creo il progetto
          current_project->create();
          loadProject();
          // scrivo sulla console
          showConsoleInfo("Creazione nuovo progetto avvenuta con successo");
    }
}

void MainWindow::loadProject(){
    // imposto il titolo
    if(current_project != nullptr){
        this->setWindowTitle("Project Manager - " + current_project->getName());
        // creo il modello file system per vedere i file
        model = new QFileSystemModel(this);
        // carico il modello per il file system
        ui->projectView->setModel(model);
        //QMessageBox *msg = new QMessageBox();
        //msg->setText("aggiungo percorso base al modello: " + current_project->getPath());
//        msg->exec();
        model->setRootPath(current_project->getPath());

        model->setReadOnly(false);
        // cancello tutte le strutture
        ui->logicStrTree->clear();
        // aggiungo il nodo root
        QTreeWidgetItem *parent = addRootItem(current_project->getName(), "Progetto corrente", ui->logicStrTree);

        // controllo se esiste una versione master
        if(current_project->hasMaster()){
            Version *master = current_project->getMasterVersion();
            // la imposto
            addItem(parent, master->getName(), "Versione master", true, ui->logicStrTree);
        }

        // scorro le versioni presenti nel progetto per aggiungere i child
        for(int count = 0; count < current_project->getVersionCount(); count++){
            // controllo che la versione esista
            Version *ver = current_project->getVersion(count);
            if(ver != nullptr){
                // aggiungo il nodo child
                addItem(parent, ver->getNumericId(), "Versione " + ver->getName(), ver->isStable(), ui->logicStrTree);
            }
        }
    }
    else{
        QMessageBox *msg = new QMessageBox();
        msg->setText("Errore: è stato chiamato il caricamento del progetto senza un progetto specificato!!");
        msg->exec();
    }
}

QTreeWidgetItem* MainWindow::addRootItem(QString name, QString description, QTreeWidget *widget){
    // creo il widget
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setText(0, name);
    item->setText(1, description);
    item->setIcon(0, QIcon(":/icons/project/Icons/project_icon.png"));
    // aggiungo l'elemento
    widget->addTopLevelItem(item);
    return item;
}

QTreeWidgetItem* MainWindow::addItem(QTreeWidgetItem *parent, QString name, QString description, bool stable, QTreeWidget *widget){
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setText(0, name);
    item->setText(1, description);
    item->setIcon(0, QIcon(":/icons/versions/Icons/root_icon.jpg"));
    if(stable){
        item->setText(2, "Si");
    }
    else{
        item->setText(2, "No");
    }
    parent->addChild(item);

    return item;
}

void MainWindow::showConsoleInfo(QString info){
    ui->console->appendHtml("<br><b><font color=#0000FF>" + info + "</font> </b>");
}

void MainWindow::showConsoleError(QString error){
    ui->console->appendHtml("<br><b><font color=#FF0000>" + error + "</font> </b>");
}

void MainWindow::showConsoleWarning(QString text){
    ui->console->appendHtml("<br><b><font color=#FFFB11>" + text + "</font> </b>");
}

void MainWindow::on_actionVersione_master_triggered()
{
    if(current_project != nullptr){
        // crea una nuova versione master da cui copiare le altre
        MasterVersionWin *wn = new MasterVersionWin(current_project);
        // eredito lo stile
        wn->setStyleSheet(dark_style_sheet);

        int res = wn->exec();
        if(res == QDialog::Accepted){
            Version *version = wn->getVersion();
            current_project->setMasterVersion(version);
            // mostro le informazioni
            QMessageBox *info = new QMessageBox();
            info->setText("Impostata versione master: \nNome versione: " + version->getName() + "\nPercorso: " + version->getCreationPath() + "\nId numerico: " + version->getNumericId());
            info->setIcon(QMessageBox::Information);
            info->exec();

            // aggiorno il progetto
            loadProject();
        }
    }
    else{
        QMessageBox::critical(this, "Errore", "Impossibile creare versione senza un progetto. ");
    }
}

void MainWindow::on_actionAggiungi_versione_triggered()
{
    // controllo se è stato specificato il progetto
    if(current_project != nullptr){
        NewVersionWin *win = new NewVersionWin(current_project);
        win->setStyleSheet(dark_style_sheet);
        int res = win->exec();
        if(res == QDialog::Accepted){
            // aggiungo la versione al progetto
            Version *ver = win->getVersion();
            current_project->addVersion(ver);
            // creo la versione
            if(win->fromMaster()){
                // creo la versione dal master
                ver->createOnDisk(current_project->getMasterVersion());
            }
            else{
                // creo la versione dalla versione precedente
                ver->createOnDisk(current_project->getVersion(current_project->getVersionCount() - 2));
            }
            // carico il progetto
            loadProject();
        }
    }
    else{
        // do errore
        QMessageBox *msg = new QMessageBox();
        msg->setText("Nessun progetto creato. Impossibile aggiungere una versione");
        msg->setIcon(QMessageBox::Information);
        msg->exec();
    }


}

bool MainWindow::existsProject(QString why){
    // controllo se esiste
    if(current_project != nullptr){
        return true;
    }
    else{
        // mostro errore
        QMessageBox *msg = new QMessageBox();
        msg->setText("Errore: " + why);
        msg->setIcon(QMessageBox::Critical);
        msg->exec();
        return false;
    }
}

void MainWindow::updateLogics(QTreeWidgetItem *current)
{
    if(current != nullptr && current_project != nullptr){
        qInfo() << "Aggiorno item selezionato. Testo descrizione: " << current->text(1) << endl;
            // controllo su quale versione devo lavorare
            Version *version = nullptr;

            QString text = ui->logicStrTree->currentItem()->text(1);

            if(text != ""){
                if(text.contains("progetto", Qt::CaseInsensitive)){
                    // niente
                }
                else if(text.contains("master", Qt::CaseInsensitive)){
                    showConsoleInfo("Aggiorno versione master");
                    version = current_project->getMasterVersion();
                }
                else if(text.contains("versione", Qt::CaseInsensitive)){
                    showConsoleInfo("aggiorno versione numero: " + QString::number(ui->logicStrTree->currentIndex().row() - 1));
                    version = current_project->getVersion(ui->logicStrTree->currentIndex().row() - 1);
                }

                if(version != nullptr){
                    // pulisco la lista delle entry
                    ui->objectsView->clear();
                    // avvio la ricerca
                    VersionFileParser parser;

                    // imposto il percorso
                    parser.setSearchPath(version->getCreationPath());
                    parser.setConsoleSurf(ui->console);

                    if(parser.execute()){
                        showConsoleInfo("Success");

                        // estraggo i valori trovati e li metto nella lista
                        QStringList results = parser.getResults();

                        // scorro e aggiungo alla lista
                        for(int x = 0; x < results.count(); x++){
                            ui->objectsView->addItem(results[x]);
                        }

                        // imposto il numero di file
                        ui->filesNum->setText("Numero file presenti: " + QString::number(results.count()));
                    }
                    else{
                        showConsoleError("Errore");
                    }

                    ui->versionLogicView->clear();
                    // cerco le classi
                    LogicObjectParser par2;
                    par2.setSearchPath(version->getCreationPath());
                    par2.setConsole(ui->console);
                    par2.setSurface(ui->versionLogicView);
                    par2.setWindowInfo(this, ui->progressBar);
                    // creo una palette con i colori da utilizzare

                    par2.search(dark_theme_parser);
                }
            }
    }
}



void MainWindow::openProjectInExplorer(){
    // controllo che il progetto esista
    if(existsProject(" impossibile aprire il percorso di un progetto non esistente.")){
        // procedo
        // creo un processo
        // creo la lista di argomenti
        QStringList list;
        list.append(current_project->getProjFilePath());
        // creo il processo
        QProcess *process = new QProcess(this);
        process->start("EXPLORER.EXE", list);
    }
}

void MainWindow::openMasterInExplorer(){
    // controllo che il progetto esista
    if(existsProject("impossibile aprire versione master da un progetto non esistente")){
        // creo una lista con l'argomento del percorso della versione master
        QStringList list;
        list.append(current_project->getMasterVersion()->getCreationPath());

        // creo un processo e lo avvio
        QProcess *process = new QProcess(this);
        process->start("EXPLORER.EXE", list);
    }
}

void MainWindow::openVersionInExplorer(unsigned int num){
    if(existsProject("impossibile aprire una versione da un progetto non esistente")){
        // creo una lista con l'argomento del percorso della versione master
        QStringList list;
        list.append(current_project->getVersion(num)->getCreationPath());

        // creo un processo e lo avvio
        QProcess *process = new QProcess(this);
        process->start("EXPLORER.EXE", list);
    }
}

void MainWindow::openCmdPrompt(){
    system(("cmd.exe /K C:\\Users"));
}

void MainWindow::openCmdPrompt(QString path){
    system(("cmd.exe /K " + path).toStdString().c_str());
}

void MainWindow::on_openInExplorerBtn_clicked()
{
    if(current_project != nullptr){
        // apro l'elemento selezionato
        // controllo la seconda colonna dell' elemento
        // salvo il testo della seconda colonna del controllo
        QString descr = ui->logicStrTree->currentItem()->text(1);
        // converto la stringa in caratteri maiuscoli
        descr = descr.toUpper();
        // controllo se contiene la parola progetto
        if(descr.contains("PROGETTO")){
            // si tratta di un progetto
            // apro il percorso del progetto corrente in explorer
            openProjectInExplorer();
        }
        else if(descr.contains("MASTER")){
            // apro la versione master nell' explorer
            openMasterInExplorer();
        }
        else if(descr.contains("VERSIONE")){
            openVersionInExplorer(ui->logicStrTree->currentIndex().row() - 1);
        }
        else{

        }
    }
}


void MainWindow::on_actionStruttura_file_in_explorer_triggered()
{
    on_openInExplorerBtn_clicked();
}

void MainWindow::on_actionConsole_di_sistema_triggered()
{
    openCmdPrompt();
}

void MainWindow::on_openInTerminalBtn_clicked()
{
    if(current_project != nullptr){
        // apro l'elemento selezionato
        // controllo la seconda colonna dell' elemento
        // salvo il testo della seconda colonna del controllo
        QString descr = ui->logicStrTree->currentItem()->text(1);
        // converto la stringa in caratteri maiuscoli
        descr = descr.toUpper();
        // controllo se contiene la parola progetto
        if(descr.contains("PROGETTO")){
            // si tratta di un progetto
            // apro il percorso del progetto corrente in explorer
            openCmdPrompt(current_project->getProjFilePath());
        }
        else if(descr.contains("MASTER")){
            // apro la versione master nell' explorer
            openCmdPrompt(current_project->getMasterVersion()->getCreationPath());
        }
        else if(descr.contains("VERSIONE")){
            openCmdPrompt(current_project->getVersion(ui->logicStrTree->currentIndex().row() - 1)->getCreationPath());
        }
        else{

        }
    }
}

void MainWindow::on_logicStrTree_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    updateLogics(current);
}

void MainWindow::on_pushButton_clicked()
{
    // chiamo il metodo per aggiornare
    on_logicStrTree_currentItemChanged(ui->logicStrTree->currentItem(), nullptr);
}

void MainWindow::on_openFileBtn_clicked()
{
    // apro il file
    // salvo il percorso
    QStringList args;
    QString file_path = ui->objectsView->currentItem()->text();
    args.append(file_path);
    // apro
    QProcess *process = new QProcess();
    process->setProgram("NOTEPAD");
    process->setArguments(args);
    process->start();
}

void MainWindow::on_pushButton_3_clicked()
{
    // salvo il nome del file
    QString file_name = ui->objectsView->currentItem()->text();
    // cancello il file
    if(
    deleteFile(file_name)){
        QMessageBox *msg = new QMessageBox();
        msg->setText("Rimozione file " + file_name + " avvenuta con successo.");
        msg->setIcon(QMessageBox::Information);
        msg->exec();
        // aggiorno la visualizzazione
        on_logicStrTree_currentItemChanged(ui->logicStrTree->currentItem(), nullptr);
    }
    else{
        QMessageBox *msg = new QMessageBox();
        msg->setText("Rimozione file " + file_name + " non riuscita");
        msg->setIcon(QMessageBox::Critical);
        msg->exec();
    }
}

void MainWindow::on_actionRimuovi_versione_triggered()
{
    if(current_project != nullptr){
        // controllo se si tratta di una versione
        QString text = ui->logicStrTree->currentItem()->text(1);

        if(text.contains("versione", Qt::CaseInsensitive)){
            // cancello la versione
            if(
            current_project->deleteVersion(ui->logicStrTree->currentIndex().row() - 1)){
                // controllo il risultato
                QMessageBox *msg = new QMessageBox();
                msg->setText("Rimozione versione " + ui->logicStrTree->currentItem()->text(1) + " avvenuta con successo");
                msg->setIcon(QMessageBox::Information);
                msg->exec();
                loadProject();
            }
            else{
                QMessageBox *msg = new QMessageBox();
                msg->setText("Impossibile rimuovere la versione " + ui->logicStrTree->currentItem()->text(1));
                msg->setIcon(QMessageBox::Critical);
                msg->exec();
            }
        }
        else if(text.contains("master", Qt::CaseInsensitive)){
            // cancello la master
            //current_project->getMasterVersion()->deleteFromDisk();
            // aggiorno il progetto
            loadProject();
        }
        else{
            // niente
        }
    }
    else{
        QMessageBox::critical(this, "Errore", "Impossibile rimuovere una versione senza un  progetto.");
    }
}

void MainWindow::on_actionApri_versione_master_triggered()
{
    if(current_project != nullptr){
        openMasterInExplorer();
    }
    else{
        QMessageBox::critical(this, "Errore", "Nessun progetto contenente versioni. ");
    }
}

void MainWindow::on_newVersionBtn_clicked()
{
    // avvio la azione
    ui->actionAggiungi_versione->trigger();
}

void MainWindow::on_deleteObjectBtn_clicked()
{
    if(current_project != nullptr){
        on_actionRimuovi_versione_triggered();
    }
    else{
        QMessageBox::critical(this, "Errore", "Nessun progetto contenente versioni. ");
    }
}


void MainWindow::on_actionSalva_triggered()
{
    // salvo
    current_project->save();
}

void MainWindow::on_addClassBtn_clicked()
{
    if(current_project != nullptr){
            NewClassWin *win = new NewClassWin(current_project);
            // imposto il tema
            if(dark_style_sheet != ""){
                win->setStyleSheet(dark_style_sheet);
            }
            int res = win->exec();

            if(res == QDialog::Accepted){
                ui->versionLogicView->addTopLevelItem(win->getResult()->formatForView());
            }
    }
    else{
        QMessageBox::critical(this, "Errore ", "Impossibile aggiungere una classe senza un progetto. ");
    }
}

void MainWindow::on_pushButton_5_clicked()
{
    if(current_project != nullptr){
        updateLogics(ui->logicStrTree->currentItem());
    }
    else{
        QMessageBox::critical(this, "Errore",  "Impossibile aggiornare vista logica senza un progetto. ");
    }
}

void MainWindow::on_versionLogicView_customContextMenuRequested(const QPoint &pos)
{
    // creo il menu contestuale
    QMenu *context = new QMenu(ui->versionLogicView);
    // aggiungo una azione
    context->addAction(ui->actionMostra_informazioni);
    context->addSeparator();
    context->addAction(ui->actionApri_dichiarazione);
    context->addAction(ui->actionApri_implementazione);
    context->addAction(ui->actionApri_percorso);
    context->addSeparator();
    context->addAction(ui->actionNuova_classe);
    context->addAction(ui->actionNuovo_sorgente);
    context->addSeparator();
    context->addAction(ui->actionElimina);
    context->addSeparator();
    context->addAction(ui->actionImporta_2);
    context->addAction(ui->actionImporta_da_versione);
    context->addAction(ui->actionEsporta_2);
    context->addSeparator();
    context->addAction(ui->actionSalva_indice);
    context->addSeparator();
    context->addAction(ui->actionEspandi_selezione);
    context->addAction(ui->actionEspandi_tutto);
    context->addAction(ui->actionChiudi_selezione);
    context->addAction(ui->actionChiudi_tutto);
    // eseguo in base alla posizione del cursore
    context->exec(ui->versionLogicView->mapToGlobal(pos));
}

void MainWindow::on_actionChiudi_tutto_triggered()
{
    ui->versionLogicView->collapseAll();
}

void MainWindow::on_actionChiudi_selezione_triggered()
{
    ui->versionLogicView->collapseItem(ui->versionLogicView->currentItem());
}

void MainWindow::on_actionEspandi_tutto_triggered()
{
    ui->versionLogicView->expandAll();
}

void MainWindow::on_actionEspandi_selezione_triggered()
{
    ui->versionLogicView->expandItem(ui->versionLogicView->currentItem());
}

void MainWindow::on_actionNuova_classe_triggered()
{
    on_addClassBtn_clicked();
}

void MainWindow::on_logicStrTree_customContextMenuRequested(const QPoint &pos)
{
    QMenu *context = new QMenu(ui->logicStrTree);
    context->addAction(ui->actionInformazioni_versione);
    context->addAction(ui->actionModifica_versione);
    context->addSeparator();
    context->addAction(ui->actionAggiungi_versione);
    context->addAction(ui->actionNuova_versione_da_qui);
    context->addSeparator();
    context->addAction(ui->actionCancella);
    context->addSeparator();
    context->addAction(ui->actionImporta);
    context->addAction(ui->actionEsporta);
    context->addSeparator();
    context->addAction(ui->actionApri_in_explorer);
    context->addAction(ui->actionApri_in_terminale);
    context->addAction(ui->actionApri_file_di_versione);
    context->addAction(ui->actionApri_file_progetto);
    context->addSeparator();

    context->addAction(ui->actionModifica_note_versione);
    context->addAction(ui->actionImposta_note_versione);
    context->addAction(ui->actionCancella_note_versione);

    context->addSeparator();

    context->addAction(ui->actionFlag_instabile);
    context->addAction(ui->actionFlag_stabile);

    context->exec(ui->logicStrTree->mapToGlobal(pos));
}

Version* MainWindow::getCurrentVersion(bool exclude_master)
{
    // determina se l'elemento corrente nell' albero del progetto è una versione
    // controllo se è stato impostato un progetto
    if(current_project != nullptr){
        // controllo che ci siano elementi
        // imposto la versione selezionata come stabile
        QString cur_txt = ui->logicStrTree->currentItem()->text(1);
        cur_txt = cur_txt.toUpper();
        // controllo se si tratta di una versione
        if(cur_txt.contains("VERSIONE")){
            // calcolo l'indice della versione
            int ver_ind = ui->logicStrTree->currentIndex().row() - 1;
            // controllo se esiste
            if(current_project->existsVersion(ver_ind)){
                return current_project->getVersion(ver_ind);
            }
            else{
                qInfo() << "Errore calcolo indice. " << ver_ind << endl;
                return nullptr;
            }
        }
        else{
            return nullptr;
        }
    }
    else{
        return nullptr;
    }
}

void MainWindow::on_actionFlag_stabile_triggered()
{
    // Leggo la versione corrente
    Version* cur = getCurrentVersion(false);
    // controllo che non sia nulla
    if(cur != nullptr){
        // la contrassegno come stabile
        cur->setStable(true);
        ui->logicStrTree->currentItem()->setText(2, "Si");
    }
    else{
        // do errore
        QMessageBox::critical(this, "Errore", "Impossibile impostare la versione come stabile. ");
    }
}

void MainWindow::on_actionFlag_instabile_triggered()
{
    // Leggo la versione corrente
    Version* cur = getCurrentVersion(false);
    // controllo che non sia nulla
    if(cur != nullptr){
        // la contrassegno come stabile
        cur->setStable(false);
        ui->logicStrTree->currentItem()->setText(2, "No");
    }
    else{
        // do errore
        QMessageBox::critical(this, "Errore", "Impossibile impostare la versione come stabile. ");
    }
}