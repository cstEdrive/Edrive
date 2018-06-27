function frprocch(inp);
% frprocch.m laat de gebruiker een keuze doen uit verschillende soorten 
%	mechanika. Tevens is er de mogelijkheid om een eigen overdr. functie
%	in te geven.
%
%	Advies Pepijn:
%		Bij mechanika met grote verschillen in de eigenwaarden (spoles.m),
%		dus bijvoorbeeld twee dubbele integratoren en een gedempte
%		trilling (massa-veer-demper-massa) is het slim om apart naar
%		de integratoren en naar de gedempte trillingen te kijken:
%		dus kijken naar de gemiddelde verplaatsing en naar de 
%		verschil verplaatsing.
%		Voor een vierde orde systeem (als boven beschreven) waarbij
%		de toestanden de [v1 x1 v2 x2]' zijn is de transformatie:
%		A~=T\A*T (T=1 0 0.5 0;0 1 0 0.5;1 0 -0.5 0;0 1 0 -0.5] nuttig;
%		is in feite (inv(T)*A*T).
%		B~ moet dan worden: B~=T\B
%		C~ = C*T en D~ = D
%		want:
%		x' = A x + B u
%		y  = C x + D u
%
%		x~	= inv(T)*x; = T\x;		(x = T*x~)
%		dus voorvermenigvuldigen met inv(T)
%		x~' = T\A T*x	+ T\B
%		y   = C   T*x   + D;
%  
%		Hierdoor verkrijgt men de toestanden: 
%		[(v1-v2) (x1-x2) (v1+v2)/2 (x1+x2)/2]'

% file           : frinitfr.m
% last changes   : november 26, 1998
% author         : M.L.Norg
% editor         : M.V.P.Kruger
% used functions : -
% version	     : 3.1
% notes	     : made file FRF-data compatible (26/11/'98)
% called from    : frdiet

global  NameMain PROCTYPE Sproc DoRecalc DoRedraw NeedTime NeedFreq ...
        NeedCont RESPPROC hzreek compPROC magnPROC phasPROC
global BUTTON V1 V2 V3 V4
%new
global frf_hz FRFyes frf_N column_meas RESP_onedec decimation

ProcStr1	= 'Mass (F-in,x-out)';
ProcStr2	= 'Damped mass (F-in,x-out)';
ProcStr3	= 'Mass (F-in,x-out) damper-spring mass';
ProcStr4	= 'Mass (F-in) damper-spring mass (x-out)';
ProcStr5	= 'Mass on frame';
ProcStr6	= 'User Defined Process';
ProcStr7    = 'Frequency Response Function (FRF)';

procchoise	= menu('Process to be controlled:',...
		       ['1/ ',ProcStr1],...
		       ['2/ ',ProcStr2],...
		       ['3/ ',ProcStr3],...
		       ['4/ ',ProcStr4],...
             ['5/ ',ProcStr5],...
		       ['6/ ',ProcStr6],...
		       ['7/ ',ProcStr7]);

T		= [1 0 0.5 0;0 1 0 0.5;1 0 -0.5 0;0 1 0 -0.5];

NameMain	= eval(['ProcStr',num2str(procchoise)]);

%procchoise
%procchoise = 3;	% tijdelijk voor debuggen;
FRFyes = 0;                 % this flag indicates whetehr FRF-data is used instead of state-space systems

if procchoise == 1,			% 2e orde (massa)
  PROCTYPE	= 'MECHAN#001';
  [BUTTON,V1]	= question(['Mass',10,'(F-in,x-out)'],'OK','Mass [kg]/[kgm2]:','1');
  Sproc		= pck([0 0;1 0],[1/str2num(V1) 0]',[0 1],[0]);

elseif procchoise == 2,			% 2e orde (massa veer demper)
  PROCTYPE	= 'MECHAN#002';
  [BUTTON,V1,V2]= question(['Damped mass',10,'(F-in,x-out)'],'OK','Mass [kg]/[kgm2]:','1','Damping [Ns/m]/[Nms/rad]:','1');
  Sproc		= tf2sys([0 0 1],[str2num(V1) str2num(V2) 0]);

%  PROCDEN	= [1/((2*pi*10)^2) 2*0.1/(2*pi*10) 1];

elseif procchoise == 3,			% Mass (F-in,x-out) damper-spring mass
  PROCTYPE	= 'MECHAN#003';
  m1		= 4.8e-6;
  d		= 1e-4;
  c		= 0.22;
  m2		= 4.8e-6;
  INPS		= 0.02;
  OUTPS		= 1;
  [BUTTON,V1,V2,V3,V4]= question(['Mass1-damper-spring-mass2',10,'(F1-in,x1-out]'],'OK|Scaling','Mass1 [kg]/[kgm2]:',num2str(m1),'Damping [Ns/m]/[Nms/rad]:',num2str(d),'Stiffness [N/m]/[Nm/rad]',num2str(c),'Mass2 [kg]/[kgm2]:',num2str(m2));
  if BUTTON == 2,
    [BUTTON1,INPS,OUTPS]= question(['Scaling'],'OK|Cancel','Input scaling [-]:',num2str(INPS),'Output scaling [-]:',num2str(OUTPS));
    if BUTTON1 == 2,
      INPS	= 1;
      OUTPS	= 1;
    else,
      INPS	= str2num(INPS);
      OUTPS	= str2num(OUTPS);
    end;
  else,
    INPS	= 1;
    OUTPS	= 1;
  end;

  m1		= str2num(V1);
  d		= str2num(V2);
  c		= str2num(V3);
  m2		= str2num(V4);
  Sproc		= pck(T\[-d/m1 -c/m1 d/m1 c/m1;1 0 0 0;d/m2 c/m2 -d/m2 -c/m2;0 0 1 0]*T,T\[1/m1*INPS 0 0 0]',[0 1*OUTPS 0 0]*T,[0]);

elseif procchoise == 4,
  PROCTYPE	= 'MECHAN#004';
  m1		= 4.8e-6;
  d		= 1e-4;
  c		= 0.22;
  m2		= 4.8e-6;
  INPS		= 0.02;
  OUTPS		= 1;
  [BUTTON,V1,V2,V3,V4]= question(['Mass1-damper-spring-mass2',10,'(F1-in,x2-out]'],'OK|Scaling','Mass1 [kg]/[kgm2]:',num2str(m1),'Damping [Ns/m]/[Nms/rad]:',num2str(d),'Stiffness [N/m]/[Nm/rad]',num2str(c),'Mass2 [kg]/[kgm2]:',num2str(m2));
  m1		= str2num(V1);
  d		= str2num(V2);
  c		= str2num(V3);
  m2		= str2num(V4);
  if BUTTON == 2,
    [BUTTON1,INPS,OUTPS]= question(['Scaling'],'OK|Cancel','Input scaling [-]:',num2str(INPS),'Output scaling [-]:',num2str(OUTPS));
    if BUTTON1 == 2,
      INPS	= 1;
      OUTPS	= 1;
    else,
      INPS	= str2num(INPS);
      OUTPS	= str2num(OUTPS);
    end;
  else,
    INPS	= 1;
    OUTPS	= 1;
  end;
  Sproc		= pck(T\[-d/m1 -c/m1 d/m1 c/m1;1 0 0 0;d/m2 c/m2 -d/m2 -c/m2;0 0 1 0]*T,T\[(1/m1)*INPS 0 0 0]',[0 0 0 1*OUTPS]*T,[0]);

elseif procchoise == 5,
  PROCTYPE	= 'MECHAN#005';
  [BUTTON,V1,V2,V3,V4]= question('Mass on frame','OK','Mass1 [kg]/[kgm2]:','100','Mass of Frame [kg]/[kgm2]:','1','Damping Frame to World [Ns/m]/[Nms/rad]:','1','Stiffness Frame to World [N/m]/[Nm/rad]','1');

  m1		= str2num(V1);
  d		= str2num(V3);
  c		= str2num(V4);
  m2		= str2num(V2);

  Sproc		= pck([0 0 0 0;1 0 0 0;0 0 -d/m2 -c/m2;0 0 1 0],[1/m1 0 -1/m2 0]',[0 1 0 -1],[0]);
elseif procchoise == 6,
  PROCTYPE	= 'MECHAN#006';

  BUTTON1 	= menu('Select type of process notation:',...
		       '1/ State Space notation [A B;C D]',...
		       '2/ Transfer Function notation [NUM / DEN]',...
		       '3/ Varying Matrix [S]',...
		       '4/ Simulink moded [filename.m]');

  if BUTTON1 == 1,

    [BUTTON,V1,V2,V3,V4]= question(['State Space Notation',10,'(Variables must be present in Workspace & GLOBALS)'],'OK|Cancel','A-matrix:','Ap','B-matrix','Bp','C-matrix','Cp','D-matrix','Dp');

    eval(['global ',V1,' ',V2,' ',V3,' ',V4]);
%    isglobal(Ap)
    if BUTTON == 1,
      if exist(V1)~=1 | exist(V2)~=1 | exist(V3)~=1 | exist(V4)~=1,
        question(['State Space Matrices not (all) found!',10,...
                  'Keyboard on CapsLock?',10,...
                  'Are matrices GLOBAL?',10,...
                  'Program will quit!'],'OK');
        exitsave; break;
      else
        eval(['At=',V1,';Bt=',V2,';Ct=',V3,';Dt=',V4,';']);
%        At
%        Bt 
%        Ct
%        Dt
        Sproc = snsys(At,Bt,Ct,Dt);

%        eval(['Sproc = snsys(',V1,',',V2,',',V3,',',V4,');']);
%        [type,orde,noi,noo]=minfo(Sproc);
        [noo,noi]	= size(Dt);
        if noi~=1 | noo~=1,		% noi en noo
          goon	= 0;
          while ~goon,
            [BUTTON2,V1,V2]=question(['State Space system has ',10,num2str(noi),' input(s) and ',num2str(noo),' ouput(s).',10,'Select one input and one output:'],'OK|Cancel',['Input # [1 - ',num2str(noi),']'],'1',['Output # [1 - ',num2str(noo),']'],'1');
            if BUTTON2 == 1,
              takeinpu	= str2num(V1);
              takeoutp	= str2num(V2);
              if ((takeinpu > 0) & (takeinpu <= noi) & (takeoutp > 0) & (takeoutp <= noo)),
                goon	= 1;
              else,
                question(['Input # or Output # out of range!',10,'Try again...'],'OK');
              end;
            elseif BUTTON2 == 2,
              exitsave; break;
            end;
          end;
	else
          takeinpu	= 1;
          takeoutp	= 1;
	end;
        Sproc = snsys(At,Bt(:,takeinpu),Ct(takeoutp,:),Dt(takeoutp,takeinpu));
      end;
    end;
  elseif BUTTON1 == 2,
    [BUTTON,V1,V2]= question(['Transfer function notation',10,'(Variables must be present in Workspace & GLOBALS)'],'OK|Cancel','Numerator:','NUMp','Denominator','DENp');

    eval(['global ',V1,' ',V2]);
    if BUTTON == 1,
      if exist(V1)~=1 | exist(V2)~=1,
        question(['Transfer Function not (completly) found!',10,'Keyboard on CapsLock?',10,'Program will quit!'],'OK');
        exitsave; break;
      else
        eval(['Sproc = nd2sys(',V1,',',V2,');']);
        [type,orde,noi,noo]=minfo(Sproc);
        if noi~=1&noo~=1,
          question(['Process has ',num2str(noi),' input(s) and ',num2str(noo),' ouput(s).',10,'Both must be 1',10,'Program will quit!'],'OK');
          exitsave; break
        end;
      end;
    end;
  elseif BUTTON1 == 3,
    [BUTTON,V1,V2]= question(['SYSTEM Matrix (S) notation',10,'(Variable must be present in Workspace & GLOBALS)'],'OK|Cancel','SYSTEM matrix:','Ssyst');
    eval(['global ',V1]);

    if BUTTON == 1,
      if exist(V1)~=1,
        question(['SYSTEM Matrix ''',V1,''' not found!',10,'Keyboard on CapsLock?',10,'Program will quit!'],'OK');
        exitsave; break;
      else
        eval(['Sproc = ',V1,';']);
        [type,orde,noi,noo]=minfo(Sproc);
        if noi~=1&noo~=1,
          question(['Process has ',num2str(noi),' input(s) and ',num2str(noo),' ouput(s).',10,'Both must be 1',10,'Program will quit!'],'OK');
          exitsave; break;
        end;
      end;
    end;
  elseif BUTTON1 == 4,
    curdir	= cd;
    [BUTTON3]=question(['Load parameters',10,'Run parameter file'],'Load *.mat|Run *.m');
    if BUTTON3 == 1,
      [parafile , parapath]	= uigetfile('*.mat','Load parameter file:');
      load(parafile);
    elseif BUTTON3 == 2,
      [parafile , parapath]	= uigetfile('*.m','Run parameter file:');
      save tempfile
      eval(lower(parafile(1:length(parafile)-2)));
      load tempfile
    end;       
                 
    [modefile , modepath]	= uigetfile('*.m','Use Simulink model:');
    if modefile ~= 0,
      BUTTON	= 1;
      modefile	= lower(modefile);
%      modepath	= lower(modepath);
      if findstr(modefile,'.m')~=[],
        modefile = modefile(1:length(modefile)-2);
      end;

      % Op de PC zit al een check of de file bestaat.
      % Op UNIX niet, dus daar moet gechecked worden! 
      if ~strcmp(computer,'PCWIN') & (exist([modepath,modefile]) ~= 2),
        [BUTTON2]	= question('File does not exists!','Quit');
      end;
      cd(modepath(1:length(modepath)-1));
      eval(['[sizes]=',modefile,';']);
      noi	= sizes(4);
      noo	= sizes(3);
      if noi~=1 | noo~=1,		% noi en noo
        goon	= 0;
        while ~goon,
          [BUTTON2,V1,V2]=question(['Simulink model ''',modefile,''' has ',10,num2str(noi),' input(s) and ',num2str(noo),' ouput(s).',10,'Select one input and one output:'],'OK|Cancel|Show model',['Input # [1 - ',num2str(noi),']'],'1',['Output # [1 - ',num2str(noo),']'],'1');
          if BUTTON2 == 1,
            takeinpu	= str2num(V1);
            takeoutp	= str2num(V2);
            if ((takeinpu > 0) & (takeinpu <= noi) & (takeoutp > 0) & (takeoutp <= noo)),
              goon	= 1;
            else,
              question(['Input # or Output # out of range!',10,'Try again...'],'OK');
            end;
          elseif BUTTON2 == 2,
            exitsave; break;
          elseif BUTTON2 == 3, 
            open_system(modefile)
          end;
        end;
      else
        takeinpu	= 1;
        takeoutp	= 1;
      end;
      [Atemp,Btemp,Ctemp,Dtemp] = linmod(modefile);
      Sproc	= sysbalr(snsys(Atemp,Btemp(:,takeinpu),Ctemp(takeoutp,:),Dtemp(takeoutp,takeinpu)));
      disp('Model gelineariseerd');
    else,
      BUTTON	= 2;
    end;
    eval(['cd ',curdir,';']);
    clear curdir;
  end;
  if BUTTON == 2, 
    question('Program will quit!','OK');
    exitsave; break;
  end;
  
elseif procchoise == 7,			% FRF-data
  PROCTYPE	= 'FRFDAT#001';
  [BUTTON,V1,V2]= question(['FRFdata',10,'(Variables must be present in workspace & GLOBALS)'],'OK|Cancel','Complex column-vector/matrix:','frf','frequency [Hz]:','hz');
  
  eval(['global ',V1,' ',V2]);
  if BUTTON == 1,
     if isempty(eval(V1)) | isempty(eval(V2))
        question(['Error: FRF-data not (completely) found!',10,'Keyboard on CapsLock?',10,'Are variables GLOBAL?',10,'Program will quit!'],'OK');
        break
     else
        eval(['compPROC = ',V1,';']);
        eval(['frf_hz = ',V2,';']);
        frf_N = size(compPROC,2);
        if size(frf_hz,1)<size(frf_hz,2)
		     frf_hz = frf_hz';
		  end
		  RESPPROC = mvfr2vmu(compPROC,frf_hz);
		  FRFyes = 1;
		  Sproc = [];
     end;
  elseif BUTTON == 2, 
    question('Program will quit!','OK');
    exitsave; break;
  end;
end;
  
DoRecalc		= 2;
DoRedraw		= 0;
NeedTime		= 1;    % een tijdrespberekening is weer nodig
NeedFreq		= 1;    % een freqresponsie berekening is weer nodig
NeedCont		= 1;	% een nieuwe regelaar moet gemaakt worden
if FRFyes ~= 1;
   RESPPROC		= frsp(Sproc,2*pi*hzreek,0);
   compPROC		= vmu2mvfr(sel(RESPPROC,1,1));
   [magnPROC,phasPROC] 	= r2p(compPROC);
elseif FRFyes == 1
   column_meas = 1;
   decimation = 1;
	fronedec
end

% Copyright Philips Research Labs, 1995,  All Rights Reserved
   
