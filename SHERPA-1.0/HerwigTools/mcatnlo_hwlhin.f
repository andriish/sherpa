C Collects all of the Les Houches interface routines, plus utilities
C for colour codes
C
C----------------------------------------------------------------------
      SUBROUTINE UPEVNTHW
C----------------------------------------------------------------------
C  Reads MC@NLO input files and fills Les Houches event common HEPEUP
C----------------------------------------------------------------------
      INCLUDE 'HERWIG65.INC'
C---Les Houches Event Common Block
      INTEGER MAXNUP
      PARAMETER (MAXNUP=500)
      INTEGER NUP,IDPRUP,IDUP,ISTUP,MOTHUP,ICOLUP
      DOUBLE PRECISION XWGTUP,SCALUP,AQEDUP,AQCDUP,PUP,VTIMUP,SPINUP,
     & XMP2,XMA2,XMB2,BETA,VA,VB,SIGMA,DELTA,S2,XKA,XKB,PTF,E,PL
      COMMON/HEPEUP/NUP,IDPRUP,XWGTUP,SCALUP,AQEDUP,AQCDUP,
     &              IDUP(MAXNUP),ISTUP(MAXNUP),MOTHUP(2,MAXNUP),
     &              ICOLUP(2,MAXNUP),PUP(5,MAXNUP),VTIMUP(MAXNUP),
     &              SPINUP(MAXNUP)
      DOUBLE PRECISION PCM(5),PTR,XMTR,HWULDO
      INTEGER I,J,IC,JPR,IHVY,MQQ,NQQ,IUNIT,ISCALE,I1HPRO,IBOS,
     & ILEP,ID,IA,IB,ICOL4(4,4),ICOL5(5,18),JJPROC
      PARAMETER (IUNIT=61)
      LOGICAL BOPRO,NODEC,REMIT
      COMMON/NQQCOM/MQQ,NQQ
C---Colour flows for heavy quark pair production
      DATA ICOL4/
     & 10,02,10,02,01,20,20,01,12,23,10,03,12,31,30,02/
      DATA ICOL5/
     & 10,02,13,30,02, 10,02,32,10,03,
     & 10,21,30,20,03, 10,23,20,10,03,
     & 01,20,23,30,01, 01,20,31,20,03,
     & 01,23,03,20,01, 01,12,03,30,02,
     & 12,20,30,10,03, 12,30,10,30,02,
     & 12,03,02,10,03, 12,01,03,30,02,
     & 12,23,14,40,03, 12,34,32,10,04,
     & 12,23,43,10,04, 12,31,34,40,02,
     & 12,34,14,30,02, 12,31,42,30,04/
      IF (IERROR.NE.0) RETURN
C---READ AN EVENT
      IF(NQQ.GE.MQQ)CALL HWWARN('UPEVNT',201,*999)
      NQQ=NQQ+1
      NUP=5
C---CHECK PROCESS CODE
      JJPROC=MOD(ABS(IPROC),10000)
      JPR=JJPROC/100
      BOPRO=JPR.EQ.13.OR.JPR.EQ.14.OR.JPR.EQ.16.OR.JPR.EQ.36
      IF (BOPRO) THEN
C----------------------------------------------------------------------
C   SINGLE GAUGE OR HIGGS BOSON PRODUCTION
C   B = Z/gamma, W or H (SM or any MSSM neutral Higgs)
C-----------------------------------------------------------------------
C I1HPRO IDENTIFIES THE PARTONIC SUBPROCESS, WITH THE FOLLOWING CONVENTIONS:
C   I1HPRO         PROCESS
C    401        q qbar -> g B
C    402        q g    -> q B
C    403        qbar q -> g B
C    404        qbar g -> qbar B
C    405        g q    -> q B
C    406        g qbar -> qbar B
C    407        g g    -> g B
C-----------------------------------------------------------------------
C---NODEC=.TRUE. FOR HIGGS AND UNDECAYED EW BOSON
         NODEC=(JPR.EQ.16) .OR.
     #         ( (JPR.EQ.13.OR.JPR.EQ.14).AND.
     #           ((JJPROC/10-10*JPR).EQ.9) )
         IF (NODEC) NUP=4
         READ(IUNIT,901) I1HPRO,(IDUP(I),I=1,3)
         IHPRO=I1HPRO-400
         ISCALE=0
         IF(JPR.EQ.16)ISCALE=2
      ELSEIF (JPR.EQ.17) THEN
C----------------------------------------------------------------------
C   HEAVY Q QBAR PRODUCTION
C   IPROC=-1705,-1706 for Q=b,t
C-----------------------------------------------------------------------
C I1HPRO IDENTIFIES THE PARTONIC SUBPROCESS, WITH THE FOLLOWING CONVENTIONS:
C   I1HPRO         PROCESS
C    401        q qbar -> g Q Qbar
C    402        q g    -> q Q Qbar
C    403        qbar q -> g Q Qbar
C    404        qbar g -> qbar Q Qbar
C    405        g q    -> q Q Qbar
C    406        g qbar -> qbar Q Qbar
C    407        g g    -> g Q Qbar
C-----------------------------------------------------------------------
C IC SPECIFIES THE COLOUR CONNECTION (NOW IN INPUT FILE)
C-----------------------------------------------------------------------
         READ(IUNIT,901) I1HPRO,(IDUP(I),I=1,3),IC
C---SET IHPRO AS FOR DIRECT PHOTON (IPROC=1800)
         IHPRO=I1HPRO-360
         ISCALE=0
         IF(ABS(IPROC).EQ.1705.OR.ABS(IPROC).EQ.11705)ISCALE=5
      ELSEIF (JPR.EQ.28) THEN
C----------------------------------------------------------------------
C   GAUGE BOSON PAIR PRODUCTION
C   VV=WW,ZZ,ZW+,ZW- FOR IPROC=-2850,-2860,-2870,-2880
C-----------------------------------------------------------------------
C I1HPRO IDENTIFIES THE PARTONIC SUBPROCESS, WITH THE FOLLOWING CONVENTIONS:
C   I1HPRO         PROCESS
C    401        q qbar -> g V V
C    402        q g    -> q V V
C    403        qbar q -> g V V
C    404        qbar g -> qbar V V
C    405        g q    -> q V V
C    406        g qbar -> qbar V V
C-----------------------------------------------------------------------
         READ(IUNIT,901) I1HPRO,(IDUP(I),I=1,3)
         IHPRO=I1HPRO-400
         ISCALE=0
      ELSE
         CALL HWWARN('UPEVNT',202,*999)
      ENDIF
      READ(IUNIT,902) XWGTUP
C---Les Houches expects mean weight to be the cross section in pb
      XWGTUP= XWGTUP*MQQ
      READ(IUNIT,903) ((PUP(J,I),J=1,4),I=1,2)
      READ(IUNIT,904) ((PUP(J,I),J=1,4),I=3,NUP)
      DO I=1,NUP
         CALL HWUMAS(PUP(1,I))
      ENDDO
      CALL HWVSUM(4,PUP(1,1),PUP(1,2),PCM)
      CALL HWUMAS(PCM)
C---HARD SCALE
      SCALUP=PCM(5)
C---REMIT MEANS A REAL PARTON EMISSION OCCURRED
      REMIT=PUP(4,3).NE.ZERO
      IF (REMIT) THEN
         IF (ISCALE.EQ.0) THEN
            PTR=SQRT(PUP(1,3)**2+PUP(2,3)**2)
            SCALUP=PCM(5)-2.*PTR
         ELSEIF(ISCALE.EQ.1)THEN
            SCALUP=PCM(5)
         ELSEIF (ISCALE.EQ.2) THEN
            SCALUP=SQRT(PUP(1,3)**2+PUP(2,3)**2)
         ELSEIF (ISCALE.EQ.3.OR.ISCALE.EQ.4.OR.ISCALE.EQ.5) THEN
            PTR=SQRT(PUP(1,3)**2+PUP(2,3)**2)
            IA=4
            IB=5
            XMP2=PUP(5,3)**2
            XMA2=PUP(5,IA)**2
            XMB2=PUP(5,IB)**2
            S2=XMA2+XMB2+2*HWULDO(PUP(1,IA),PUP(1,IB))
            SIGMA=XMA2+XMB2
            DELTA=XMA2-XMB2
            BETA=SQRT(1-2*SIGMA/S2+(DELTA/S2)**2)
            VA=BETA/(1+DELTA/S2)
            VB=BETA/(1-DELTA/S2)
            XKA=HWULDO(PUP(1,3),PUP(1,IA))
            XKB=HWULDO(PUP(1,3),PUP(1,IB))
            E=(XKA+XKB)/SQRT(S2)
            PL=-2.0/((VA+VB)*BETA*SQRT(S2))*(VA*XKA-VB*XKB)
            PTF=E**2-PL**2-XMP2
            IF (PTF.LE.ZERO) CALL HWWARN('UPEVNT',103,*999)
            PTF=SQRT(PTF)
            IF(ISCALE.EQ.3)THEN
              SCALUP=PCM(5)-2.*MIN(PTR,PTF)
            ELSEIF(ISCALE.EQ.4)THEN
              SCALUP=MIN(PTR,PTF)
            ELSE
              SCALUP=(MIN(PTR,PTF))**2+(XMA2+XMB2)/2.D0
              SCALUP=SQRT(SCALUP)
            ENDIF
            IF (SCALUP.LE.ZERO) CALL HWWARN('UPEVNT',100,*999)
         ELSEIF (ISCALE.EQ.6) THEN
            XMTR=SQRT(PUP(5,4)**2+PUP(1,4)**2+PUP(2,4)**2)
            PTR=SQRT(PUP(1,3)**2+PUP(2,3)**2)
            SCALUP=PCM(5)-PTR-XMTR
            IF (SCALUP.LE.ZERO) CALL HWWARN('UPEVNT',100,*999)
         ELSEIF (ISCALE.EQ.7) THEN
            SCALUP=SQRT(PUP(5,4)**2+PUP(1,4)**2+PUP(2,4)**2)
         ELSE
            CALL HWWARN('UPEVNT',501,*999)
         ENDIF
      ELSE
         NUP=NUP-1
      ENDIF
C---INITIAL STATE
      DO I=1,2
         ISTUP(I)=-1
         MOTHUP(1,I)=0
         MOTHUP(2,I)=0
      ENDDO
C---FINAL STATE
      DO I=3,NUP
         ISTUP(I)=1
         MOTHUP(1,I)=1
         MOTHUP(2,I)=2
      ENDDO
      IF (BOPRO.AND.NODEC) THEN
C---SINGLE BOSON (UNDECAYED)
         IF (REMIT) THEN
C---SET COLOUR CONNECTIONS
            DO I=1,3
               ICOLUP(1,I)=501
               ICOLUP(2,I)=502
            ENDDO
            IF (IHPRO.EQ.1) THEN
               ICOLUP(2,1)=0
               ICOLUP(1,2)=0
            ELSEIF (IHPRO.EQ.2) THEN
               ICOLUP(1,1)=502
               ICOLUP(2,1)=0
               ICOLUP(2,3)=0
            ELSEIF (IHPRO.EQ.3) THEN
               ICOLUP(1,1)=0
               ICOLUP(2,2)=0
            ELSEIF (IHPRO.EQ.4) THEN
               ICOLUP(1,1)=0
               ICOLUP(2,1)=501
               ICOLUP(1,3)=0
            ELSEIF (IHPRO.EQ.5) THEN
               ICOLUP(1,2)=502
               ICOLUP(2,2)=0
               ICOLUP(2,3)=0
            ELSEIF (IHPRO.EQ.6) THEN
               ICOLUP(1,2)=0
               ICOLUP(2,2)=501
               ICOLUP(1,3)=0
            ELSEIF (IHPRO.EQ.7) THEN
               ICOLUP(1,2)=502
               ICOLUP(2,2)=503
               ICOLUP(2,3)=503
            ELSE
               CALL HWWARN('UPEVNT',101,*999)
            ENDIF
         ELSE
            CALL HWVEQU(5,PUP(1,4),PUP(1,3))
C---SET COLOUR CONNECTIONS
            DO I=1,2
               ICOLUP(1,I)=0
               ICOLUP(2,I)=0
            ENDDO
            IF (IDUP(1).GT.0) THEN
               ICOLUP(1,1)=501
               ICOLUP(2,2)=501
               IF (IDUP(1).GT.0) THEN
C---GG FUSION
                  ICOLUP(2,1)=502
                  ICOLUP(1,2)=502
               ENDIF
            ELSE
C---QBAR Q
               ICOLUP(2,1)=501
               ICOLUP(1,2)=501
            ENDIF
         ENDIF
         ICOLUP(1,NUP)=0
         ICOLUP(2,NUP)=0
C---LOAD BOSON DATA
         IF (JPR.EQ.13) THEN
            IDUP(NUP)=23
         ELSEIF (JPR.EQ.16) THEN
            IDUP(NUP)=25
         ELSEIF (JPR.EQ.36) THEN
            IBOS=MOD(JJPROC,100)
            IF (IBOS.EQ.10) THEN
               IDUP(NUP)=26
            ELSEIF (IBOS.EQ.20) THEN
               IDUP(NUP)=35
            ELSEIF (IBOS.EQ.30) THEN
               IDUP(NUP)=36 
            ELSE
               CALL HWWARN('UPEVNT',502,*999)            
            ENDIF
         ELSEIF (JPR.EQ.14) THEN
            IBOS=0
            DO I=1,NUP-1
               ID=IDUP(I)
               IF (ID.EQ.21) THEN
                  IC=0
               ELSEIF (ID.GT.0) THEN
                  IC=ICHRG(ID)
               ELSE
                  IC=ICHRG(6-ID)
               ENDIF
               IBOS=IBOS+IC
            ENDDO
            IF (REMIT) IBOS=IBOS-2*IC
            IF (ABS(IBOS).NE.3) CALL  HWWARN('UPEVNT',503,*999)
            IDUP(NUP)=8*IBOS
         ENDIF
      ELSEIF (JPR.EQ.17) THEN
C---HEAVY QUARKS
         IF (REMIT) THEN
C---3-BODY FINAL STATE
C---SET COLOUR CONNECTIONS
            IF (IC.LE.18) THEN
               DO I=1,5
                  CALL UPCODE(ICOL5(I,IC),ICOLUP(1,I))
               ENDDO
            ELSE
               CALL HWWARN('UPEVNT',105,*999)
            ENDIF
         ELSE
C---2-BODY FINAL STATE
            CALL HWVEQU(5,PUP(1,4),PUP(1,3))
            CALL HWVEQU(5,PUP(1,5),PUP(1,4))
C---SET COLOUR CONNECTIONS
            IF (IC.LE.4) THEN
               DO I=1,4
                  CALL UPCODE(ICOL4(I,IC),ICOLUP(1,I))
               ENDDO
            ELSE
               CALL HWWARN('UPEVNT',104,*999)
            ENDIF
         ENDIF
         IHVY=MOD(JJPROC,100)
         IDUP(NUP-1)=IHVY
         IDUP(NUP)=-IHVY
      ELSE
C---GAUGE BOSON OR LEPTON PAIR
         IF (REMIT) THEN
C---ADD DIBOSON OR DILEPTON TO EVENT RECORD (TO FIX ITS MASS)
            NUP=6
            CALL HWVEQU(5,PUP(1,5),PUP(1,6))
            CALL HWVEQU(5,PUP(1,4),PUP(1,5))
            CALL HWVSUM(4,PUP(1,5),PUP(1,6),PUP(1,4))
            CALL HWUMAS(PUP(1,4))
            ISTUP(3)=1
            ISTUP(4)=2
            ISTUP(5)=1
            ISTUP(6)=1
            IDUP(4)=0
            MOTHUP(1,3)=1
            MOTHUP(2,3)=2
            MOTHUP(1,4)=1
            MOTHUP(2,4)=2
            MOTHUP(1,5)=4
            MOTHUP(2,5)=4
            MOTHUP(1,6)=4
            MOTHUP(2,6)=4
C---SET COLOUR CONNECTIONS
            DO I=1,3
               ICOLUP(1,I)=501
               ICOLUP(2,I)=502
            ENDDO
            IF (IHPRO.EQ.1) THEN
               ICOLUP(2,1)=0
               ICOLUP(1,2)=0
            ELSEIF (IHPRO.EQ.2) THEN
               ICOLUP(1,1)=502
               ICOLUP(2,1)=0
               ICOLUP(2,3)=0
            ELSEIF (IHPRO.EQ.3) THEN
               ICOLUP(1,1)=0
               ICOLUP(2,2)=0
            ELSEIF (IHPRO.EQ.4) THEN
               ICOLUP(1,1)=0
               ICOLUP(2,1)=501
               ICOLUP(1,3)=0
            ELSEIF (IHPRO.EQ.5) THEN
               ICOLUP(1,2)=502
               ICOLUP(2,2)=0
               ICOLUP(2,3)=0
            ELSEIF (IHPRO.EQ.6) THEN
               ICOLUP(1,2)=0
               ICOLUP(2,2)=501
               ICOLUP(1,3)=0
            ELSE
               CALL HWWARN('UPEVNT',102,*999)
            ENDIF
            DO I=4,6
               ICOLUP(1,I)=0
               ICOLUP(2,I)=0
            ENDDO
         ELSE
            CALL HWVEQU(5,PUP(1,4),PUP(1,3))
            CALL HWVEQU(5,PUP(1,5),PUP(1,4))
C---SET COLOUR CONNECTIONS
            DO I=1,4
               ICOLUP(1,I)=0
               ICOLUP(2,I)=0
            ENDDO
            IF (IDUP(1).GT.0) THEN
               ICOLUP(1,1)=501
               ICOLUP(2,2)=501
            ELSE
               ICOLUP(2,1)=501
               ICOLUP(1,2)=501
            ENDIF
         ENDIF
         IF (BOPRO) THEN
C---DILEPTON PRODUCTION
            IBOS=MOD(JJPROC,100)
            ILEP=MOD(JJPROC,10)
            IBOS=IBOS-ILEP
C---LOAD LEPTON AND BOSON DATA
            I=NUP-1
            J=NUP
            IF ( IBOS.EQ.50 .OR.
     #           (IBOS.EQ.60.AND.JPR.EQ.13) .OR.
     #           (IBOS.EQ.70.AND.JPR.EQ.13) ) THEN
               IDUP(I)=-ILEP-10
               IDUP(J)=-IDUP(I)
               IF (REMIT) IDUP(4)=23
            ELSEIF (IBOS.EQ.60.AND.JPR.EQ.14) THEN
               IDUP(I)=-9-2*ILEP
               IDUP(J)=1-IDUP(I)
               IF (REMIT) IDUP(4)=24
            ELSEIF (IBOS.EQ.70.AND.JPR.EQ.14) THEN
               IDUP(I)=-10-2*ILEP
               IDUP(J)=-1-IDUP(I)
               IF (REMIT) IDUP(4)=-24
            ELSE
               CALL HWWARN('UPEVNT',504,*999)
            ENDIF
         ELSE
C---DIBOSON PRODUCTION
            IBOS=MOD(JJPROC,100)
C---LOAD BOSON DATA
            I=NUP-1
            J=NUP
            IF (IBOS.EQ.50) THEN
               IDUP(I)=24
               IDUP(J)=-24
            ELSEIF (IBOS.EQ.60) THEN
               IDUP(I)=23
               IDUP(J)=23
            ELSEIF (IBOS.EQ.70) THEN
               IDUP(I)=24
               IDUP(J)=23
            ELSEIF (IBOS.EQ.80) THEN
               IDUP(I)=-24
               IDUP(J)=23
            ELSE
               CALL HWWARN('UPEVNT',505,*999)
            ENDIF
         ENDIF
      ENDIF
 901  FORMAT(1X,I3,4(1X,I2))
 902  FORMAT(1X,D14.8)
 903  FORMAT(8(1X,D14.8))
 904  FORMAT(12(1X,D14.8))
 999  END
C----------------------------------------------------------------------
      SUBROUTINE UPCODE(ICODE,ICOL)
C--DECODES COLOUR CONNECTIONS
C----------------------------------------------------------------------
      IMPLICIT NONE
      INTEGER ICODE,ICOL(2)
      ICOL(1)=ICODE/10
      IF (ICOL(1).NE.0) ICOL(1)=ICOL(1)+500
      ICOL(2)=MOD(ICODE,10)
      IF (ICOL(2).NE.0) ICOL(2)=ICOL(2)+500
      END
C----------------------------------------------------------------------
      SUBROUTINE UPINITHW
C----------------------------------------------------------------------
C  Reads MC@NLO input headers and fills Les Houches run common HEPRUP
C----------------------------------------------------------------------
      INCLUDE 'HERWIG65.INC'
C--Les Houches Common Blocks
      INTEGER MAXPUP
      PARAMETER(MAXPUP=100)
      INTEGER IDBMUP,PDFGUP,PDFSUP,IDWTUP,NPRUP,LPRUP
      DOUBLE PRECISION EBMUP,XSECUP,XERRUP,XMAXUP
      COMMON /HEPRUP/ IDBMUP(2),EBMUP(2),PDFGUP(2),PDFSUP(2),
     &                IDWTUP,NPRUP,XSECUP(MAXPUP),XERRUP(MAXPUP),
     &                XMAXUP(MAXPUP),LPRUP(MAXPUP)
      INTEGER MAXNUP
      PARAMETER (MAXNUP=500)
      INTEGER NUP,IDPRUP,IDUP,ISTUP,MOTHUP,ICOLUP
      DOUBLE PRECISION XWGTUP,SCALUP,AQEDUP,AQCDUP,PUP,VTIMUP,SPINUP
      COMMON/HEPEUP/NUP,IDPRUP,XWGTUP,SCALUP,AQEDUP,AQCDUP,
     &              IDUP(MAXNUP),ISTUP(MAXNUP),MOTHUP(2,MAXNUP),
     &              ICOLUP(2,MAXNUP),PUP(5,MAXNUP),VTIMUP(MAXNUP),
     &              SPINUP(MAXNUP)
      DOUBLE PRECISION XCKECM,XTMP1,XTMP2,XTMP3,XTMP4,XMT,XMW,XMZ,
     & XMH,XMV,XM1,XM2,XM3,XM4,XM5,XM21,XLAM,GAH,TINY
      INTEGER IVVCODE,IFAIL,MQQ,NQQ,IHW,I,NDNS,JPR,JPR0,IH
      CHARACTER*60 TMPSTR
      CHARACTER*4 STRP1,STRP2
      CHARACTER*8 STRGRP
      CHARACTER*2 STRSCH
      CHARACTER*50 QQIN
      LOGICAL FK88STRNOEQ
C--VALUE CHANGED FROM 1.D-3 to 1.D-2
      DATA TINY/1.D-2/
C
      COMMON/NQQCOM/MQQ,NQQ
      COMMON/VVJIN/QQIN
C
      IF (IERROR.NE.0) RETURN
C--SET UP INPUT FILES
      OPEN(UNIT=61,FILE=QQIN,STATUS='UNKNOWN')
C--READ HEADERS OF EVENT FILE
      READ(61,801)XCKECM,XTMP1,XTMP2,XTMP3,XTMP4,TMPSTR
      READ(61,802)IVVCODE,TMPSTR
      IVVCODE=MOD(IVVCODE,10000)
C---CHECK PROCESS CODE
      JPR0=MOD(ABS(IPROC),10000)
      JPR=JPR0/100
      IF (JPR.NE.IVVCODE/100) CALL HWWARN('UPINIT',500,*999)
      IF ((JPR.EQ.17.OR.JPR.EQ.28.OR.JPR.EQ.36).AND.
     & IVVCODE.NE.MOD(ABS(IPROC),10000)) CALL HWWARN('UPINIT',501,*999)
      IF (JPR.EQ.13.OR.JPR.EQ.14) THEN
         IF(JPR0.EQ.1396.OR.JPR0.EQ.1371.OR.
     #      JPR0.EQ.1372.OR.JPR0.EQ.1373)THEN
           READ(61,808)EMMIN,EMMAX,TMPSTR
         ELSE
           READ(61,809)XMV,GAH,GAMMAX,TMPSTR
         ENDIF
C-- CHECK VECTOR BOSON MASS
         IF( (IVVCODE.EQ.1397.AND.ABS(XMV-RMASS(200)).GT.TINY) .OR.
     #       (IVVCODE.EQ.1497.AND.ABS(XMV-RMASS(198)).GT.TINY) .OR.
     #       (IVVCODE.EQ.1498.AND.ABS(XMV-RMASS(199)).GT.TINY) )
     #      CALL HWWARN('UPINIT',502,*999)
      ELSEIF (JPR.EQ.28) THEN
         READ(61,808)XMW,XMZ,TMPSTR
C-- CHECK VECTOR BOSON MASSES
         write (*,*)'masses:',xmw,xmz,rmass(198),rmass(200),tiny
         IF(ABS(XMW-RMASS(198)).GT.TINY .OR.
     #   ABS(XMZ-RMASS(200)).GT.TINY) CALL HWWARN('UPINIT',502,*999)
      ELSEIF (JPR.EQ.16.OR.JPR.EQ.36) THEN
         READ(61,809)XMH,GAH,XMT,TMPSTR
C-- CHECK HIGGS AND TOP MASSES
         IH=201
         IF (JPR.EQ.36) IH=IVVCODE/10-158
         IF(ABS(XMH-RMASS(IH)).GT.TINY) CALL HWWARN('UPINIT',503,*999)
         IF(ABS(XMT-RMASS(6)) .GT.TINY) CALL HWWARN('UPINIT',504,*999)
      ELSEIF (JPR.EQ.17) THEN
         READ(61,803)XMT,TMPSTR
C-- CHECK HEAVY QUARK MASS
         IF( (IVVCODE.EQ.1706.AND.ABS(XMT-RMASS(6)).GT.TINY) .OR.
     #       (IVVCODE.EQ.1705.AND.ABS(XMT-RMASS(5)).GT.TINY) .OR.
     #       (IVVCODE.EQ.1704.AND.ABS(XMT-RMASS(4)).GT.TINY) )
     #   CALL HWWARN('UPINIT',505,*999)
      ELSE
         CALL HWWARN('UPINIT',506,*999)
      ENDIF
      READ(61,804)XM1,XM2,XM3,XM4,XM5,XM21,TMPSTR
      READ(61,805)STRP1,STRP2,TMPSTR
      READ(61,806)STRGRP,NDNS,TMPSTR
      READ(61,807)XLAM,STRSCH,TMPSTR
C--CHECK THAT EVENT FILE HAS BEEN GENERATED CONSISTENTLY WITH 
C--HERWIG PARAMETERS ADOPTED HERE
      IFAIL=0
C-- CM ENERGY
      IF( ABS(XCKECM-PBEAM1-PBEAM2).GT.TINY .OR.
C-- QUARK AND GLUON MASSES
     #     ABS(XM1-RMASS(1)).GT.TINY .OR.
     #     ABS(XM2-RMASS(2)).GT.TINY .OR.
     #     ABS(XM3-RMASS(3)).GT.TINY .OR.
     #     ABS(XM4-RMASS(4)).GT.TINY .OR.
     #     ABS(XM5-RMASS(5)).GT.TINY .OR.
     #     ABS(XM21-RMASS(13)).GT.TINY .OR.
C-- LAMBDA_QCD: NOW REMOVED TO ALLOW MORE FLEXIBILITY (NNLO EFFECT ANYHOW)
C     #     ABS(XLAM-QCDLAM).GT.TINY .OR.
C-- REPLACE THE FOLLOWING WITH A CONDITION ON STRSCH, IF CONSISTENT 
C-- INFORMATION ON PDF SCHEME WILL BE AVAILABLE FROM PDF LIBRARIES AND HERWIG
C-- COLLIDING PARTICLE TYPE
     #     FK88STRNOEQ(STRP1,PART1) .OR.
     #     FK88STRNOEQ(STRP2,PART2) )IFAIL=1
C--IF PDF LIBRARY IS USED, CHECK PDF CONSISTENCY
      IF( IFAIL.EQ.0 .AND. MODPDF(1).NE.-1)THEN
            IF( 
     #          FK88STRNOEQ(STRGRP,AUTPDF(1)) .OR.
     #          FK88STRNOEQ(STRGRP,AUTPDF(2)) .OR.
     #          ABS(NDNS-MODPDF(1)).GT.TINY .OR.
     #          ABS(NDNS-MODPDF(2)).GT.TINY )IFAIL=1
      ENDIF
      IF(IFAIL.EQ.1) CALL HWWARN('UPINIT',507,*999)
      CALL HWUIDT(3,IDBMUP(1),IHW,PART1)
      CALL HWUIDT(3,IDBMUP(2),IHW,PART2)
      DO I=1,2
         EBMUP(I)=HALF*XCKECM
         PDFGUP(I)=-1
         PDFSUP(I)=-1
      ENDDO
      IDWTUP=-4
      NPRUP=1
      LPRUP(1)=IVVCODE
      READ(61,900) MQQ
      NQQ=0
      NUP=6
      AQEDUP=ZERO
      AQCDUP=ZERO
      DO I=1,NUP
         VTIMUP(I)=ZERO
         SPINUP(I)=9.
      ENDDO
 801  FORMAT(5(1X,D10.4),1X,A)
 802  FORMAT(1X,I6,1X,A)
 803  FORMAT(1X,D10.4,1X,A)
 804  FORMAT(6(1X,D10.4),1X,A)
 805  FORMAT(2(1X,A4),1X,A)
 806  FORMAT(1X,A8,1X,I4,1X,A)
 807  FORMAT(1X,D10.4,1X,A2,1X,A)
 808  FORMAT(2(1X,D10.4),1X,A)
 809  FORMAT(3(1X,D10.4),1X,A)
 900  FORMAT(I9)
 999  END
