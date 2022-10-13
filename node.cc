/*
 * node.cc
 *
 *  Created on: Apr 29, 2021
 *      Author: saidtig
 */




    #include <string.h>
    #include <omnetpp.h>
    #include <stdio.h>
    #include "myDeg_m.h"
    #include "myColor_m.h"
    #include "color_yourself_m.h"
    #include "dsRequest_m.h"
    #include "conflit_m.h"
    #include "alreadyColored_m.h"
    #include "dsResponse_m.h"
    #include "impass_m.h"
    #include <vector>
    #include <algorithm>
    #include <map>
    using namespace omnetpp;



    class Node : public cSimpleModule
    {
    private:
        std::vector<int> neighbors;
        std::vector<int> list_color;
        std::vector<int> actif_neighbors;
        std::vector<int> remain;
        bool start;
        bool colored;
        int deg;
        int count;
        int degMax;
        int nextGate;
        int color;
        int degSat;
        int count2;
        int dsMax;
        int master;
        int deg2;
        int degSat2;
        int impass;


        std::map <int, int> a;
        std::map <int, int> b;
        std::map <int, int> c;
        std::map <int, int> u;
        std::map <int, int> f;
        std::map <int, bool> colored_resp;
     protected:
        virtual void initialize() override;
        virtual void handleMessage(cMessage *msg) override;
        virtual MyDeg *generateMessage();
        virtual MyColor *generateMessage2();
        virtual Color_yourself *generateMessage3();
        virtual DsRequest *generateMessage4();
        virtual DsResponse *generateMessage5();
        virtual Impass *generateMessage6();
        virtual Conflit *generateMessage7();
        virtual AlreadyColored *generateMessage8();
    };


    Define_Module(Node);

    void Node::initialize()
    {
        for (int i; i<par("nbr_colors").intValue(); i++)
            list_color.push_back(i+1);
        start = true;
        deg = gateSize("gate$o");
        EV<<"Le degre de "<<getIndex()<<" est "<<deg<<"\n";
        count = 0;
        degMax = 0;
        dsMax = 0;
        count2 = 0;
        degSat = 0;
        colored = false;
        impass = 0;
        MyDeg *msg = generateMessage();
            for (int i = 0; i < gateSize("gate$o"); i++)
            {
                MyDeg *copy = msg->dup();
                send(copy, "gate$o", i);
            }
            delete msg;
        }


    void Node::handleMessage(cMessage *msg)
    {
        if (strcmp(msg->getName(),"my degre")==0){
            MyDeg *deg_msg = check_and_cast<MyDeg *>(msg);
            neighbors.push_back(int(deg_msg->getSource()));
            actif_neighbors.push_back(int(deg_msg->getSource()));
            count+=1;
            a[(deg_msg->getArrivalGate())->getIndex()] = int(deg_msg->getSource());
            u[int(deg_msg->getSource())] = (deg_msg->getArrivalGate())->getIndex();
            b[(deg_msg->getArrivalGate())->getIndex()] = int(deg_msg->getDegre());

            if(count == gateSize("gate$o")){
                for (auto &m:b){
                    if(m.second > deg) start = false;
                    else if(m.second == deg && a[m.first]>getIndex()) start = false;
                }
            }

         if(start == true && count == gateSize("gate$o")){
             bubble("START");
             colored = true;
             std::sort(list_color.begin(),list_color.end());
             color = list_color[0];
             list_color.erase(std::remove(list_color.begin(),list_color.end(),color),list_color.end());
             cDisplayString& dispStr = getDisplayString();
             if (color == 1) dispStr.parse("i=,blue");
             else if(color == 2) dispStr.parse("i=,cyan");
             else if(color == 3) dispStr.parse("i=,green");
             else if(color == 4) dispStr.parse("i=,yellow");
             else if(color == 5) dispStr.parse("i=,red");
             else if(color == 6) dispStr.parse("i=,grey");
             else if(color == 7) dispStr.parse("i=,gold");
             else if(color == 8) dispStr.parse("i=,orange");
             else if(color == 9) dispStr.parse("i=,pink");
             else if(color == 10) dispStr.parse("i=,brown");
             else if(color == 11) dispStr.parse("i=,violet");
             else if(color == 12) dispStr.parse("i=,black");
             else dispStr.parse("i=,black");

             EV<<"Le noeud "<<getIndex()<<" prend la couleur C"<<color<<". \n";

             MyColor *color_msg = generateMessage2();
             for (int i = 0; i < gateSize("gate$o"); i++)
             {
                 MyColor *copy = color_msg->dup();
                 send(copy, "gate$o", i);
             }
             EV<<"Le noeud "<<getIndex()<<" informe ses voisins qu'il a pris la couleur C"<<color<<". \n";

             for (auto &m:b){
                 //EV<<"le voisin "<<a[m.first]<< " sur gate "<< m.first<<" a un degre " <<m.second<<"\n";
                 if(m.second > degMax) {degMax = m.second; nextGate = m.first;}
                 else if(m.second == degMax && a[m.first]>a[nextGate]) nextGate = m.first;
             }
             actif_neighbors.erase(std::remove(actif_neighbors.begin(),actif_neighbors.end(),a[nextGate]),actif_neighbors.end());
             remain = actif_neighbors;
             Color_yourself *order_msg = generateMessage3();
             send(order_msg,"gate$o",nextGate);
             EV<<"Le noeud "<<getIndex()<<" ordonne au noeud "<<a[nextGate]<<" de prendre une couleur. (reste: ";
             for (auto& i:actif_neighbors) EV<<i<<", ";
             EV<<") \n";


         }
    }
        if (strcmp(msg->getName(),"my color")==0){
            MyColor *myColor_msg = check_and_cast<MyColor *>(msg);
            list_color.erase(std::remove(list_color.begin(),list_color.end(),myColor_msg->getColor()),list_color.end());
            actif_neighbors.erase(std::remove(actif_neighbors.begin(),actif_neighbors.end(),myColor_msg->getSource()),actif_neighbors.end());
            remain.erase(std::remove(remain.begin(),remain.end(),myColor_msg->getSource()),remain.end());
            degSat+=1;
        }
        if (strcmp(msg->getName(),"color yourself")==0){    // Je recoit un message color_yourself
            Color_yourself *colorYourself_msg = check_and_cast<Color_yourself *>(msg);
            if(colored == false){   //si je n'ai pas encore de couleur (pas de probleme de ds request par noeud "m" avant color yourself par noeud "n")
                colored = true;     //car je vais prendre une couleur
                master = colorYourself_msg->getArrivalGate()->getIndex();   //me servira pour passer l'impass s'il y en aura
                std::sort(list_color.begin(),list_color.end());             //pour prendre la plus petite couleur
                if (list_color.size()!=0) color = list_color[0];            //si j'ai des couleur je prend
                else {
                    color = 1;
                    Conflit *conflit_msg = generateMessage7();              // si je n'ai pas de couleur j'informe mon voisin qu'il y a un conflit
                    send(conflit_msg, "gate$o",master);
                }
//Pour l'affichage
                cDisplayString& dispStr = getDisplayString();
                if (color == 1) dispStr.parse("i=,blue");
                else if(color == 2) dispStr.parse("i=,cyan");
                else if(color == 3) dispStr.parse("i=,green");
                else if(color == 4) dispStr.parse("i=,yellow");
                else if(color == 5) dispStr.parse("i=,red");
                else if(color == 6) dispStr.parse("i=,grey");
                else if(color == 7) dispStr.parse("i=,gold");
                else if(color == 8) dispStr.parse("i=,orange");
                else if(color == 9) dispStr.parse("i=,pink");
                else if(color == 10) dispStr.parse("i=,brown");
                else if(color == 11) dispStr.parse("i=,violet");
                else if(color == 12) dispStr.parse("i=,black");
                else dispStr.parse("i=,black");
//-------------
                list_color.erase(std::remove(list_color.begin(),list_color.end(),color),list_color.end());  //je supprime la couleur de la liste
                for (int i=0; i < colorYourself_msg->getRemainArraySize(); i++) remain.push_back(colorYourself_msg->getRemain(i));
                EV<<"Le lien "<<getIndex()<<" prend la couleur C"<<color<<". \n";

                MyColor *color_msg = generateMessage2();        //j'informe mes voisins
                for (int i = 0; i < gateSize("gate$o"); i++)
                {
                    MyColor *copy = color_msg->dup();
                    send(copy, "gate$o", i);
                }
                EV<<"Le noeud virtuel "<<getIndex()<<" informe ses voisins qu'il a pris la couleur C"<<color<<". \n";

                if(actif_neighbors.size() !=0){
                    EV<<"Le noeud "<<getIndex()<<" demande les DS de ses voisins: ";
                    DsRequest *dsRequest_msg = generateMessage4();
                    for (auto& i:actif_neighbors)       //je demande les DS de mes voisins actifs si j'en ai
                    {
                        EV<<i<<", ";
                        DsRequest *copy = dsRequest_msg->dup();
                        send(copy, "gate$o", u[i]);
                    }
                    EV<<"\n";}
                else if(actif_neighbors.size() == 0 && colorYourself_msg->getRemainArraySize() !=0){
                    EV<<"Le noeud "<<getIndex()<<" n'a plus de voisin actif mais il reste des noeuds non colories (renvoi du msg impass). \n";
                    Impass *impass_msg = generateMessage6();        //je renvoi impass si j'en ai pas de voisins actifs mais reste des noeuds dans le graphe
                    send(impass_msg,"gate$o",master);
                    }
                }
            else{           //else du premier if (ligne 172)
                AlreadyColored *already_msg = generateMessage8();      //si je suis deja colorie (probleme du ds_request avant color yourself)
                send(already_msg,"gate$o",colorYourself_msg->getArrivalGate()->getIndex());
            }
        }
        if (strcmp(msg->getName(),"ds request")==0){

            DsRequest *dsReq_msg = check_and_cast<DsRequest *>(msg);
            EV<< "Le noeud "<<getIndex()<<" repond au noeud "<<dsReq_msg->getSource()<< " qu'il a un DS= "<<degSat<<"( colored= )"<<colored<<"\n";
            DsResponse *dsResponse_msg = generateMessage5();
            send(dsResponse_msg,"gate$o",dsReq_msg->getArrivalGate()->getIndex());
        }
        if (strcmp(msg->getName(),"ds response")==0){
            DsResponse *dsResp_msg = check_and_cast<DsResponse *>(msg);
            count2+=1;
            f[dsResp_msg->getArrivalGate()->getIndex()] = dsResp_msg->getDegreSat();
            colored_resp[dsResp_msg->getArrivalGate()->getIndex()] = dsResp_msg->getColored();
            if (count2 == actif_neighbors.size()){
                for(auto& m:f){
                    if(m.second > dsMax) {dsMax = m.second; nextGate = m.first;}
                    else if(m.second == dsMax && b[m.first] > b[nextGate]) {dsMax = m.second; nextGate = m.first;}
                    else if(m.second == dsMax && b[m.first] == b[nextGate]  && a[m.first] > a[nextGate]) {dsMax = m.second; nextGate = m.first;}
                }
                actif_neighbors.erase(std::remove(actif_neighbors.begin(),actif_neighbors.end(),a[nextGate]),actif_neighbors.end());
                remain.erase(std::remove(remain.begin(),remain.end(),a[nextGate]),remain.end());
                remain.insert(remain.end(),actif_neighbors.begin(),actif_neighbors.end());
                sort( remain.begin(), remain.end() );
                remain.erase(unique(remain.begin(),remain.end()),remain.end());
                Color_yourself *order_msg = generateMessage3();
                send(order_msg,"gate$o",nextGate);
                EV<<"Le noeud "<<getIndex()<<" ordonne au noeud "<<a[nextGate]<<" de prendre une couleur. (reste: ";
                for (auto& i:remain) EV<<i<<", ";
                EV<<") \n";
                count2 = 0;
                f.clear();
                colored_resp.clear();
                dsMax = 0;
            }

        }

        if (strcmp(msg->getName(),"impass")==0){
            Impass *impass_msg = check_and_cast<Impass *>(msg);
            impass++;
            remain.clear();
            for (int i=0; i < impass_msg->getRemainArraySize(); i++) remain.push_back(impass_msg->getRemain(i));
            if(actif_neighbors.size() == 0 && impass_msg->getRemainArraySize() !=0 && impass < 20){
                EV<<"Le noeud " << getIndex()<<" recoit IMPASS et le transfere a son maitre " << a[master]<<"\n";
                Impass *impassT_msg = generateMessage6();
                send(impassT_msg,"gate$o",master);
            }
            //else if(actif_neighbors.size() != 0 && actif_neighbors == 1)
            else if (actif_neighbors.size() != 0) {
                EV<<"Le noeud " << getIndex()<<" recoit IMPASS et il a des voisins actifs. \n";
                EV<<"Le noeud "<<getIndex()<<" demande les DS de ses voisins: ";
                DsRequest *dsRequest_msg = generateMessage4();
                for (auto& i:actif_neighbors)
                {
                    EV<<i<<", ";
                    DsRequest *copy = dsRequest_msg->dup();
                    send(copy, "gate$o", u[i]);
                }
                EV<<"\n";}

            }
        else if (strcmp(msg->getName(),"already_colored")==0){  //Reception d'un message already colored
            AlreadyColored *already_msg = check_and_cast<AlreadyColored *>(msg);    //supprimer le noeud des voisins actifs
            actif_neighbors.erase(std::remove(actif_neighbors.begin(),actif_neighbors.end(),already_msg->getSource()),actif_neighbors.end());
            if(already_msg->getColor() == color){   //Si mon voisin est colorié avec la meme couleur que moi (probleme de ds_request avant color_yourself)
                if(list_color.size()!=0){       // Si j'ai d'autre couleurs disponibles
                    color = list_color[0];      //Je change ma couleur pour ne plus etre en conflit avec mon voisin
                    MyColor *color_msg = generateMessage2();    //J'informe mes voisins
                    for (int i = 0; i < gateSize("gate$o"); i++)
                    {
                        MyColor *copy = color_msg->dup();
                        send(copy, "gate$o", i);
                    }
                    EV<<"Le noeud virtuel "<<getIndex()<<" informe ses voisins qu'il change sa couleur en C"<<color<<". \n";
                }

            }


            }

        }

    MyDeg *Node::generateMessage()
    {
            int source = getIndex();
            MyDeg *msg = new MyDeg("my degre");
            msg->setDegre(deg);
            msg->setSource(source);
            return msg;
    }

    MyColor *Node::generateMessage2()
    {
            int source = getIndex();
            MyColor *msg = new MyColor("my color");
            msg->setDegre(deg);
            msg->setDegreSat(degSat);
            msg->setSource(source);
            msg->setColor(color);
            return msg;
    }

    Color_yourself *Node::generateMessage3()
        {
                int source = getIndex();
                Color_yourself *msg = new Color_yourself("color yourself");
                msg->setDegre(deg);
                msg->setDegreSat(degSat);
                msg->setSource(source);
                msg->setRemainArraySize(remain.size());
                msg->setColor(color);
                for (int i; i<remain.size(); i++){
                    msg->setRemain(i, remain[i]);
                }
                return msg;
        }
    DsRequest *Node::generateMessage4()
    {
            int source = getIndex();
            DsRequest *msg = new DsRequest("ds request");
            msg->setDegre(deg);
            msg->setSource(source);
            msg->setColor(color);
            return msg;
    }
    DsResponse *Node::generateMessage5()
       {
               int source = getIndex();
               DsResponse *msg = new DsResponse("ds response");
               msg->setDegre(deg);
               msg->setDegreSat(degSat);
               msg->setSource(source);
               msg->setColored(colored);
               return msg;
       }
    Impass *Node::generateMessage6()
    {
            int source = getIndex();
            Impass *msg = new Impass("impass");
            msg->setDegre(deg);
            msg->setSource(source);
            msg->setRemainArraySize(remain.size());
            for (int i; i<remain.size(); i++){
                msg->setRemain(i, remain[i]);
            }
            return msg;
    }
    Conflit *Node::generateMessage7()
    {
            int source = getIndex();
            Conflit *msg = new Conflit("conflit");
            msg->setSource(source);
            return msg;
    }
    AlreadyColored *Node::generateMessage8()
    {
            int source = getIndex();
            AlreadyColored *msg = new AlreadyColored("already_colored");
            msg->setSource(source);
            msg->setColor(color);
            return msg;
    }
