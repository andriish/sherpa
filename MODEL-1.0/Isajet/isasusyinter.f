C
C
C
      SUBROUTINE ISASUSYINTER(XXSUGIN,XXNUSUG,XXGMIN,XXSM,
     .                        IIMODEL,FNAME)
C

C     Copied and modified from program sugrun by Frank Krauss.
C     Last modification: 10/06/03
C
C     Sugrun:
C     Main program to calculate MSSM input parameters for ISAJET
C     from renormalization group equations and supergravity.
C     All external names are of the form SUxxxx.
C     Must link with block data ALDATA.
C
C
C
      IMPLICIT  NONE
      REAL      XXSUGIN,XXNUSUG,XXGMIN,XXSM
      DIMENSION XXSUGIN(7),XXNUSUG(18),XXGMIN(14),XXSM(15)
      INTEGER   IITERATOR,IIMODEL


      COMMON/SSLUN/LOUT
      INTEGER LOUT
      SAVE /SSLUN/


      COMMON/SSSM/AMUP,AMDN,AMST,AMCH,AMBT,AMTP,AME,AMMU,AMTAU
     $,AMW,AMZ,GAMW,GAMZ,ALFAEM,SN2THW,ALFA2,ALFA3,ALQCD4
      REAL AMUP,AMDN,AMST,AMCH,AMBT,AMTP,AME,AMMU,AMTAU
     $,AMW,AMZ,GAMW,GAMZ,ALFAEM,SN2THW,ALFA2,ALFA3,ALQCD4
      SAVE /SSSM/

      COMMON/SSPAR/AMGLSS,AMULSS,AMURSS,AMDLSS,AMDRSS,AMSLSS
     $,AMSRSS,AMCLSS,AMCRSS,AMBLSS,AMBRSS,AMB1SS,AMB2SS
     $,AMTLSS,AMTRSS,AMT1SS,AMT2SS,AMELSS,AMERSS,AMMLSS,AMMRSS
     $,AMLLSS,AMLRSS,AML1SS,AML2SS,AMN1SS,AMN2SS,AMN3SS
     $,TWOM1,RV2V1,AMZ1SS,AMZ2SS,AMZ3SS,AMZ4SS,ZMIXSS(4,4)
     $,AMW1SS,AMW2SS
     $,GAMMAL,GAMMAR,AMHL,AMHH,AMHA,AMHC,ALFAH,AAT,THETAT
     $,AAB,THETAB,AAL,THETAL,AMGVSS,MTQ,MBQ,MLQ,FBMA,
     $VUQ,VDQ
      REAL AMGLSS,AMULSS,AMURSS,AMDLSS,AMDRSS,AMSLSS
     $,AMSRSS,AMCLSS,AMCRSS,AMBLSS,AMBRSS,AMB1SS,AMB2SS
     $,AMTLSS,AMTRSS,AMT1SS,AMT2SS,AMELSS,AMERSS,AMMLSS,AMMRSS
     $,AMLLSS,AMLRSS,AML1SS,AML2SS,AMN1SS,AMN2SS,AMN3SS
     $,TWOM1,RV2V1,AMZ1SS,AMZ2SS,AMZ3SS,AMZ4SS,ZMIXSS
     $,AMW1SS,AMW2SS
     $,GAMMAL,GAMMAR,AMHL,AMHH,AMHA,AMHC,ALFAH,AAT,THETAT
     $,AAB,THETAB,AAL,THETAL,AMGVSS,MTQ,MBQ,MLQ,FBMA,VUQ,VDQ
      REAL AMZISS(4)
      EQUIVALENCE (AMZISS(1),AMZ1SS)
      SAVE /SSPAR/

      INTEGER IDUP,IDDN,IDST,IDCH,IDBT,IDTP
      INTEGER IDNE,IDE,IDNM,IDMU,IDNT,IDTAU
      INTEGER IDGL,IDGM,IDW,IDZ,IDH
      PARAMETER (IDUP=1,IDDN=2,IDST=3,IDCH=4,IDBT=5,IDTP=6)
      PARAMETER (IDNE=11,IDE=12,IDNM=13,IDMU=14,IDNT=15,IDTAU=16)
      PARAMETER (IDGL=9,IDGM=10,IDW=80,IDZ=90,IDH=81)
      INTEGER ISUPL,ISDNL,ISSTL,ISCHL,ISBT1,ISTP1
      INTEGER ISNEL,ISEL,ISNML,ISMUL,ISNTL,ISTAU1
      INTEGER ISUPR,ISDNR,ISSTR,ISCHR,ISBT2,ISTP2
      INTEGER ISNER,ISER,ISNMR,ISMUR,ISNTR,ISTAU2
      INTEGER ISZ1,ISZ2,ISZ3,ISZ4,ISW1,ISW2,ISGL
      INTEGER ISHL,ISHH,ISHA,ISHC
      INTEGER ISGRAV
      INTEGER IDTAUL,IDTAUR
      PARAMETER (ISUPL=21,ISDNL=22,ISSTL=23,ISCHL=24,ISBT1=25,ISTP1=26)
      PARAMETER (ISNEL=31,ISEL=32,ISNML=33,ISMUL=34,ISNTL=35,ISTAU1=36)
      PARAMETER (ISUPR=41,ISDNR=42,ISSTR=43,ISCHR=44,ISBT2=45,ISTP2=46)
      PARAMETER (ISNER=51,ISER=52,ISNMR=53,ISMUR=54,ISNTR=55,ISTAU2=56)
      PARAMETER (ISGL=29)
      PARAMETER (ISZ1=30,ISZ2=40,ISZ3=50,ISZ4=60,ISW1=39,ISW2=49)
      PARAMETER (ISHL=82,ISHH=83,ISHA=84,ISHC=86)
      PARAMETER (ISGRAV=91)
      PARAMETER (IDTAUL=10016,IDTAUR=20016)


      COMMON /SUGMG/ MSS(32),GSS(31),MGUTSS,GGUTSS,AGUTSS,FTGUT,
     $FBGUT,FTAGUT,FNGUT
      REAL MSS,GSS,MGUTSS,GGUTSS,AGUTSS,FTGUT,FBGUT,FTAGUT,FNGUT
      SAVE /SUGMG/

      COMMON /SUGXIN/ XISAIN(24),XSUGIN(7),XGMIN(14),XNRIN(4)
      REAL XISAIN,XSUGIN,XGMIN,XNRIN
      SAVE /SUGXIN/


      COMMON /SUGPAS/ XTANB,MSUSY,AMT,MGUT,MU,G2,GP,V,VP,XW,
     $A1MZ,A2MZ,ASMZ,FTAMZ,FBMZ,B,SIN2B,FTMT,G3MT,VEV,HIGFRZ,
     $FNMZ,AMNRMJ,NOGOOD,IAL3UN,ITACHY,MHPNEG,ASM3
      REAL XTANB,MSUSY,AMT,MGUT,MU,G2,GP,V,VP,XW,
     $A1MZ,A2MZ,ASMZ,FTAMZ,FBMZ,B,SIN2B,FTMT,G3MT,VEV,HIGFRZ,
     $FNMZ,AMNRMJ,ASM3
      INTEGER NOGOOD,IAL3UN,ITACHY,MHPNEG
      SAVE /SUGPAS/


      COMMON /SUGNU/ XNUSUG(18)
      REAL XNUSUG
      SAVE /SUGNU/


      COMMON/ISAPW/ISAPW1
      CHARACTER*30 ISAPW1
      SAVE /ISAPW/


      CHARACTER*80 FNAME
      REAL M0,MHF,A0,TANB,SGNMU,MT,XLAMGM,XMESGM,XN5GM,AMPL,XCMGV
      INTEGER NSTEP,IMODEL,INUSUG,IMODIN
      INTEGER K,NOUT,IALLOW,IITEST,J
      CHARACTER*40 VERSN,VISAJE


      PARAMETER (NOUT=33)
      INTEGER IDOUT(NOUT)
      CHARACTER*30 ISAPW2
      SAVE ISAPW2
      DATA IDOUT/
     $IDTP,ISGL,ISUPL,ISDNL,ISSTL,ISCHL,ISBT1,ISTP1,ISUPR,ISDNR,
     $ISSTR,ISCHR,ISBT2,ISTP2,ISEL,ISMUL,ISTAU1,ISNEL,ISNML,ISNTL,
     $ISER,ISMUR,ISTAU2,ISZ1,ISZ2,ISZ3,ISZ4,ISW1,ISW2,
     $ISHL,ISHH,ISHA,ISHC/
      DATA AMPL/2.4E18/
      DATA ISAPW2/'ALDATA REQUIRED BY FORTRAN G,H'/


C
C          Initialize
C
C
C          Initialize standard model parameters in /SSSM/:
C
      AMUP   = XXSM(1)
      AMDN   = XXSM(2)
      AMST   = XXSM(3)
      AMCH   = XXSM(4)
      AMBT   = XXSM(5)
      AMTP   = XXSM(6)
      AMT    = XXSM(6)
      AME    = XXSM(7)
      AMMU   = XXSM(8)
      AMTAU  = XXSM(9)
      AMZ    = XXSM(10)
      GAMW   = XXSM(11)
      GAMZ   = XXSM(12)
      ALFAEM = XXSM(13)
      SN2THW = XXSM(14)
      ALFA2  = ALFAEM/SN2THW
      ALQCD4 = 0.177
      ALFA3  = XXSM(15)
C
      PRINT *,'SM parameters :'
      PRINT *,'Yukawas   : ',AMUP,',',AMDN,',',AMST
      PRINT *,'Yukawas   : ',AMCH,',',AMBT,',',AMTP
      PRINT *,'Yukawas   : ',AME,',',AMMU,',',AMTAU
      PRINT *,'Gauge     : ',AMZ,',',GAMW,',',GAMZ
      PRINT *,'Couplings : ',ALFAEM,',',SN2THW,',',ALFA3
C
      IF(ISAPW1.NE.ISAPW2) THEN
        PRINT*, ' ERROR: BLOCK DATA ALDATA HAS NOT BEEN LOADED.'
        PRINT*, ' ISAJET CANNOT RUN WITHOUT IT.'
        PRINT*, ' PLEASE READ THE FINE MANUAL FOR ISAJET.'
        STOP99
      ENDIF
C
      LOUT     = 1
      NSTEP    = 1000
      XNRIN(2) = 1.E20
      
C      OPEN(1,FILE=FNAME,STATUS='NEW',FORM='FORMATTED')
      
      IMODEL   = IIMODEL

      IF (IMODEL.EQ.4) THEN
        IAL3UN=1
        IMODEL=1
      END IF

      IF (IMODEL.EQ.1.OR.IMODEL.EQ.3.OR.IMODEL.EQ.6) THEN
         M0    = XXSUGIN(1)
         MHF   = XXSUGIN(2)
         A0    = XXSUGIN(3)
         TANB  = XXSUGIN(4)
         SGNMU = XXSUGIN(5)
         MT    = XXSUGIN(6)
         AMGVSS = 1.0E+20 
        IF (IMODEL.EQ.6) THEN
          IMODEL=1
          PRINT*,'Error in Interface to Isajet :'
          PRINT*,'Models with Neutrino not supported by Sherpa so far'
          GO TO 15
        END IF
        IF (IMODEL.EQ.3) THEN
          IMODEL=1
          DO IITERATOR=1,18
             XNUSUG(IITERATOR) = XXNUSUG(IITERATOR)
          ENDDO
        END IF
      ELSE IF (IMODEL.EQ.2.OR.IMODEL.EQ.5) THEN
         M0       = XXSUGIN(1)
         MHF      = XXSUGIN(2)
         A0       = XXSUGIN(3)
         TANB     = XXSUGIN(4)
         SGNMU    = XXSUGIN(5)
         MT       = XXSUGIN(6)
         XGMIN(7) = XXSUGIN(7)
         XGMIN(8) = 1.
         AMGVSS   = M0*MHF*XCMGV/SQRT(3.)/AMPL

         IF (IMODEL.EQ.5) THEN
            DO IITERATOR=8,14
               XGMIN(IITERATOR)  = XXGMIN(IITERATOR)
            ENDDO
         END IF
      ELSE IF (IMODEL.EQ.7) THEN
         M0       = XXSUGIN(1)
         MHF      = XXSUGIN(2)
         A0       = 0.D0
         TANB     = XXSUGIN(4)
         SGNMU    = XXSUGIN(5)
         MT       = XXSUGIN(6)
      ELSE
        PRINT*,'Invalid model choice : ',IMODEL
        STOP99
      END IF
C
C          Solve RG equations
C
      PRINT *,'CALL SUGRA(',M0,'/',MHF,'/',A0,'/',
     .     TANB,'/',SGNMU,'/',MT,'/',IMODEL,')'
15    CALL SUGRA(M0,MHF,A0,TANB,SGNMU,MT,IMODEL)
C
C          Print results
C

      VERSN=VISAJE()
      WRITE(LOUT,20) VERSN
20    FORMAT(' ',44('*')/' *',42X,'*'/
     $  ' * ',A40,' *'/
     $  ' *',42X,'*'/' ',44('*')/)
      IF (NOGOOD.EQ.1) THEN
        PRINT*, 'BAD POINT: TACHYONIC PARTICLES!'
        WRITE(LOUT,*) 'BAD POINT: TACHYONIC PARTICLES!'
      ELSE IF (NOGOOD.EQ.2) THEN
        PRINT*, 'BAD POINT: NO EW SYMMETRY BREAKING!'
        WRITE(LOUT,*) 'BAD POINT: NO EW SYMMETRY BREAKING!'
      ELSE IF (NOGOOD.EQ.3) THEN
        PRINT*, 'BAD POINT: M(H_P)^2<0!'
        WRITE(LOUT,*) 'BAD POINT: M(H_P)^2<0!'
      ELSE IF (NOGOOD.EQ.4) THEN
        PRINT*, 'BAD POINT: YUKAWA>10!'
        WRITE(LOUT,*) 'BAD POINT: YUKAWA>10!'
      ELSE IF (NOGOOD.EQ.5.AND.IMODEL.EQ.1) THEN
        PRINT*, 'SUGRA BAD POINT: Z1SS NOT LSP!'
        WRITE(LOUT,*) 'SUGRA BAD POINT: Z1SS NOT LSP!'
      ELSE IF (NOGOOD.EQ.7) THEN
        PRINT*, 'BAD POINT: XT EWSB BAD!'
        WRITE(LOUT,*) 'BAD POINT: XT EWSB BAD!'
      ELSE IF (NOGOOD.EQ.8) THEN
        PRINT*, 'BAD POINT: MHL^2<0!'
        WRITE(LOUT,*) 'BAD POINT: MHL^2<0!'
      ELSE IF (NOGOOD.EQ.-1) THEN
        PRINT*, 'BAD POINT: NO RGE SOLUTION FOUND'
        WRITE(LOUT,*) 'BAD POINT: NO RGE SOLUTION FOUND'
      END IF
      IF (MHPNEG.EQ.1) THEN
        PRINT*, 'BAD POINT: M(H_P)^2<0!!'
        WRITE(LOUT,*) 'BAD POINT: M(H_P)^2<0!!'
        NOGOOD=3
      END IF
      IF(NOGOOD.NE.0) STOP99
      IF(ITACHY.NE.0) THEN
        WRITE(LOUT,*) 'WARNING: TACHYONIC SLEPTONS AT GUT SCALE'
        WRITE(LOUT,*) '         POINT MAY BE INVALID'
      ENDIF
C
C          Print selected model and results
C
      IF(IMODIN.EQ.1) WRITE(LOUT,1001)
1001  FORMAT(//' Minimal supergravity (mSUGRA) model:'/)
      IF(IMODIN.EQ.2) WRITE(LOUT,1002)
1002  FORMAT(//' Minimal gauge mediated (GMSB) model:'/)
      IF(IMODIN.EQ.3) WRITE(LOUT,1003)
1003  FORMAT(//' Non-universal supergravity model:'/)
      IF(IMODIN.EQ.4) WRITE(LOUT,1004)
1004  FORMAT(//' Supergravity model with truly unified couplings:'/)
      IF(IMODIN.EQ.5) WRITE(LOUT,1005)
1005  FORMAT(//' Non-minimal gauge mediated (GMSB) model:'/)
      IF(IMODIN.EQ.6) WRITE(LOUT,1006)
1006  FORMAT(//' Supergravity model with right-handed neutrinos:'/)
      IF(IMODIN.EQ.7) WRITE(LOUT,1007)
1007  FORMAT(//' Anomaly-mediated SUSY breaking model:'/)
      CALL SUGPRT(IMODEL,IMODIN)
C
C          Calculate all masses and decay modes
C
        CALL SSMSSM(XISAIN(1),XISAIN(2),XISAIN(3),
     $ XISAIN(4),XISAIN(5),XISAIN(6),XISAIN(7),XISAIN(8),XISAIN(9),
     $ XISAIN(10),XISAIN(11),XISAIN(12),XISAIN(13),XISAIN(14),
     $ XISAIN(15),XISAIN(16),XISAIN(17),XISAIN(18),XISAIN(19),
     $ XISAIN(20),XISAIN(21),XISAIN(22),XISAIN(23),XISAIN(24),
     $ MT,IALLOW,IMODEL)
C
C          Test parameters
C
      IF(IALLOW.NE.0) THEN
        WRITE(LOUT,2001)
2001    FORMAT(//' MSSM WARNING: Z1SS IS NOT LSP')
      ENDIF
C
      CALL SSTEST(IALLOW)
      IITEST=IALLOW/2
      IF(MOD(IITEST,2).NE.0) THEN
        WRITE(LOUT,2002)
2002    FORMAT(' MSSM WARNING: Z -> Z1SS Z1SS EXCEEDS BOUND')
      ENDIF
      IITEST=IITEST/2
      IF(MOD(IITEST,2).NE.0) THEN
        WRITE(LOUT,2004)
2004    FORMAT(' MSSM WARNING: Z -> CHARGINOS ALLOWED')
      ENDIF
      IITEST=IITEST/2
      IF(MOD(IITEST,2).NE.0) THEN
        WRITE(LOUT,2008)
2008    FORMAT(' MSSM WARNING: Z -> Z1SS Z2SS TOO BIG')
      ENDIF
      IITEST=IITEST/2
      IF(MOD(IITEST,2).NE.0) THEN
        WRITE(LOUT,2016)
2016    FORMAT(' MSSM WARNING: Z -> SQUARKS, SLEPTONS ALLOWED')
      ENDIF
      IITEST=IITEST/2
      IF(MOD(IITEST,2).NE.0) THEN
        WRITE(LOUT,2032)
2032    FORMAT(' MSSM WARNING: Z -> Z* HL0 EXCEEDS BOUND')
      ENDIF
      IITEST=IITEST/2
      IF(MOD(IITEST,2).NE.0) THEN
        WRITE(LOUT,2064)
2064    FORMAT(' MSSM WARNING: Z -> HL0 HA0 ALLOWED')
      ENDIF
C
      WRITE(LOUT,3600)
3600  FORMAT(//' ISASUSY decay modes:'/
     $' Parent --> daughters',18X,'Width',10X,'Branching ratio'/)
C          Write all modes
      DO 200 J=1,NOUT
        CALL SSPRT(IDOUT(J))
200   CONTINUE
C
      END


C================================================================
C
C
      SUBROUTINE FILLDECAYS(MOTHER,TOTALWIDTH,NDECAYS,
     &                      DAUGHTERS,ND,PARTIALWIDTH)

      
      INTEGER MXSS
      PARAMETER (MXSS=1000)
      COMMON/SSMODE/NSSMOD,ISSMOD(MXSS),JSSMOD(5,MXSS),GSSMOD(MXSS)
     $,BSSMOD(MXSS),MSSMOD(MXSS),LSSMOD
      INTEGER NSSMOD,ISSMOD,JSSMOD,MSSMOD
      REAL GSSMOD,BSSMOD
      LOGICAL LSSMOD
      SAVE /SSMODE/
C
      INTEGER ID,I,K,NOUT,MOTHER,NDECAYS
      INTEGER DAUGHTERS(300),ND(100)
      REAL    TOTALWIDTH,PARTIALWIDTH(100)
      CHARACTER*5 SSID,LBLIN,LBLOUT(3)

      NDECAYS    = 0
      TOTALWIDTH = 0.D0
      DO 100 I=1,NSSMOD
        IF(ISSMOD(I).NE.MOTHER) GO TO 100
        NDECAYS               = NDECAYS+1
        PARTIALWIDTH(NDECAYS) = GSSMOD(I)
        TOTALWIDTH            = TOTALWIDTH+GSSMOD(I) 
        LBLIN=SSID(ISSMOD(I))
        DO 110 K=1,3
        LBLOUT(K)=SSID(JSSMOD(K,I))
110     DAUGHTERS(3*(NDECAYS-1)+K)  = JSSMOD(K,I)    
C        Print *, LBLIN,' ',(LBLOUT(K),K=1,3),' ',GSSMOD(I),' ',BSSMOD(I)
100   CONTINUE

      END

C================================================================
C
C
      SUBROUTINE CHARGINO(MCH1,MCH2,GamL,GamR,THX,THY)

      COMMON/SSPAR/AMGLSS,AMULSS,AMURSS,AMDLSS,AMDRSS,AMSLSS
     $,AMSRSS,AMCLSS,AMCRSS,AMBLSS,AMBRSS,AMB1SS,AMB2SS
     $,AMTLSS,AMTRSS,AMT1SS,AMT2SS,AMELSS,AMERSS,AMMLSS,AMMRSS
     $,AMLLSS,AMLRSS,AML1SS,AML2SS,AMN1SS,AMN2SS,AMN3SS
     $,TWOM1,RV2V1,AMZ1SS,AMZ2SS,AMZ3SS,AMZ4SS,ZMIXSS(4,4)
     $,AMW1SS,AMW2SS
     $,GAMMAL,GAMMAR,AMHL,AMHH,AMHA,AMHC,ALFAH,AAT,THETAT
     $,AAB,THETAB,AAL,THETAL,AMGVSS,MTQ,MBQ,MLQ,FBMA,
     $VUQ,VDQ
      REAL AMGLSS,AMULSS,AMURSS,AMDLSS,AMDRSS,AMSLSS
     $,AMSRSS,AMCLSS,AMCRSS,AMBLSS,AMBRSS,AMB1SS,AMB2SS
     $,AMTLSS,AMTRSS,AMT1SS,AMT2SS,AMELSS,AMERSS,AMMLSS,AMMRSS
     $,AMLLSS,AMLRSS,AML1SS,AML2SS,AMN1SS,AMN2SS,AMN3SS
     $,TWOM1,RV2V1,AMZ1SS,AMZ2SS,AMZ3SS,AMZ4SS,ZMIXSS
     $,AMW1SS,AMW2SS
     $,GAMMAL,GAMMAR,AMHL,AMHH,AMHA,AMHC,ALFAH,AAT,THETAT
     $,AAB,THETAB,AAL,THETAL,AMGVSS,MTQ,MBQ,MLQ,FBMA,VUQ,VDQ
      REAL AMZISS(4)
      EQUIVALENCE (AMZISS(1),AMZ1SS)
      SAVE /SSPAR/
 
      REAL MCH1,MCH2,GamL,GamR,ThX,ThY


C CHECK SIGN CONVENTION   

      MCH1 = AMW1SS
      MCH2 = AMW2SS
      GamL = GAMMAL
      GamR = GAMMAR

      ThX = SIGN(1.0D0,1.0D0/TAN(GamL))
      ThY = SIGN(1.0D0,1.0D0/TAN(GamR))
C
      END
C================================================================
C
C
      SUBROUTINE NEUTRALINO(MN,NMIX)

      COMMON/SSPAR/AMGLSS,AMULSS,AMURSS,AMDLSS,AMDRSS,AMSLSS
     $,AMSRSS,AMCLSS,AMCRSS,AMBLSS,AMBRSS,AMB1SS,AMB2SS
     $,AMTLSS,AMTRSS,AMT1SS,AMT2SS,AMELSS,AMERSS,AMMLSS,AMMRSS
     $,AMLLSS,AMLRSS,AML1SS,AML2SS,AMN1SS,AMN2SS,AMN3SS
     $,TWOM1,RV2V1,AMZ1SS,AMZ2SS,AMZ3SS,AMZ4SS,ZMIXSS(4,4)
     $,AMW1SS,AMW2SS
     $,GAMMAL,GAMMAR,AMHL,AMHH,AMHA,AMHC,ALFAH,AAT,THETAT
     $,AAB,THETAB,AAL,THETAL,AMGVSS,MTQ,MBQ,MLQ,FBMA,
     $VUQ,VDQ
      REAL AMGLSS,AMULSS,AMURSS,AMDLSS,AMDRSS,AMSLSS
     $,AMSRSS,AMCLSS,AMCRSS,AMBLSS,AMBRSS,AMB1SS,AMB2SS
     $,AMTLSS,AMTRSS,AMT1SS,AMT2SS,AMELSS,AMERSS,AMMLSS,AMMRSS
     $,AMLLSS,AMLRSS,AML1SS,AML2SS,AMN1SS,AMN2SS,AMN3SS
     $,TWOM1,RV2V1,AMZ1SS,AMZ2SS,AMZ3SS,AMZ4SS,ZMIXSS
     $,AMW1SS,AMW2SS
     $,GAMMAL,GAMMAR,AMHL,AMHH,AMHA,AMHC,ALFAH,AAT,THETAT
     $,AAB,THETAB,AAL,THETAL,AMGVSS,MTQ,MBQ,MLQ,FBMA,VUQ,VDQ
      REAL AMZISS(4)
      EQUIVALENCE (AMZISS(1),AMZ1SS)
      SAVE /SSPAR/

      REAL MN(4),NMIX(4,4)
      INTEGER I,J

C CHECK SIGN CONVENTION   

      MN(1) = AMZ1SS
      MN(2) = AMZ2SS
      MN(3) = AMZ3SS
      MN(4) = AMZ4SS

      DO I=1,4
        DO J=1,4
          NMIX(I,J) = ZMIXSS(I,J)
        ENDDO
      ENDDO

      END
C================================================================
C
C
      SUBROUTINE SNEUTRINO(MN1,MN2,MN3)

      COMMON/SSPAR/AMGLSS,AMULSS,AMURSS,AMDLSS,AMDRSS,AMSLSS
     $,AMSRSS,AMCLSS,AMCRSS,AMBLSS,AMBRSS,AMB1SS,AMB2SS
     $,AMTLSS,AMTRSS,AMT1SS,AMT2SS,AMELSS,AMERSS,AMMLSS,AMMRSS
     $,AMLLSS,AMLRSS,AML1SS,AML2SS,AMN1SS,AMN2SS,AMN3SS
     $,TWOM1,RV2V1,AMZ1SS,AMZ2SS,AMZ3SS,AMZ4SS,ZMIXSS(4,4)
     $,AMW1SS,AMW2SS
     $,GAMMAL,GAMMAR,AMHL,AMHH,AMHA,AMHC,ALFAH,AAT,THETAT
     $,AAB,THETAB,AAL,THETAL,AMGVSS,MTQ,MBQ,MLQ,FBMA,
     $VUQ,VDQ
      REAL AMGLSS,AMULSS,AMURSS,AMDLSS,AMDRSS,AMSLSS
     $,AMSRSS,AMCLSS,AMCRSS,AMBLSS,AMBRSS,AMB1SS,AMB2SS
     $,AMTLSS,AMTRSS,AMT1SS,AMT2SS,AMELSS,AMERSS,AMMLSS,AMMRSS
     $,AMLLSS,AMLRSS,AML1SS,AML2SS,AMN1SS,AMN2SS,AMN3SS
     $,TWOM1,RV2V1,AMZ1SS,AMZ2SS,AMZ3SS,AMZ4SS,ZMIXSS
     $,AMW1SS,AMW2SS
     $,GAMMAL,GAMMAR,AMHL,AMHH,AMHA,AMHC,ALFAH,AAT,THETAT
     $,AAB,THETAB,AAL,THETAL,AMGVSS,MTQ,MBQ,MLQ,FBMA,VUQ,VDQ
      REAL AMZISS(4)
      EQUIVALENCE (AMZISS(1),AMZ1SS)
      SAVE /SSPAR/

      REAL MN1,MN2,MN3
      
      MN1 = AMN1SS
      MN2 = AMN2SS
      MN3 = AMN3SS

      END
C================================================================
C
C
      SUBROUTINE HIGGSES(Mhl,MHH,MA0,MHplus,tanb,alpha,VEV)
      
      COMMON/SSPAR/AMGLSS,AMULSS,AMURSS,AMDLSS,AMDRSS,AMSLSS
     $,AMSRSS,AMCLSS,AMCRSS,AMBLSS,AMBRSS,AMB1SS,AMB2SS
     $,AMTLSS,AMTRSS,AMT1SS,AMT2SS,AMELSS,AMERSS,AMMLSS,AMMRSS
     $,AMLLSS,AMLRSS,AML1SS,AML2SS,AMN1SS,AMN2SS,AMN3SS
     $,TWOM1,RV2V1,AMZ1SS,AMZ2SS,AMZ3SS,AMZ4SS,ZMIXSS(4,4)
     $,AMW1SS,AMW2SS
     $,GAMMAL,GAMMAR,AMHL,AMHH,AMHA,AMHC,ALFAH,AAT,THETAT
     $,AAB,THETAB,AAL,THETAL,AMGVSS,MTQ,MBQ,MLQ,FBMA,
     $VUQ,VDQ
      REAL AMGLSS,AMULSS,AMURSS,AMDLSS,AMDRSS,AMSLSS
     $,AMSRSS,AMCLSS,AMCRSS,AMBLSS,AMBRSS,AMB1SS,AMB2SS
     $,AMTLSS,AMTRSS,AMT1SS,AMT2SS,AMELSS,AMERSS,AMMLSS,AMMRSS
     $,AMLLSS,AMLRSS,AML1SS,AML2SS,AMN1SS,AMN2SS,AMN3SS
     $,TWOM1,RV2V1,AMZ1SS,AMZ2SS,AMZ3SS,AMZ4SS,ZMIXSS
     $,AMW1SS,AMW2SS
     $,GAMMAL,GAMMAR,AMHL,AMHH,AMHA,AMHC,ALFAH,AAT,THETAT
     $,AAB,THETAB,AAL,THETAL,AMGVSS,MTQ,MBQ,MLQ,FBMA,VUQ,VDQ
      REAL AMZISS(4)
      EQUIVALENCE (AMZISS(1),AMZ1SS)
      SAVE /SSPAR/


      REAL Mhl,MHh,MA0,MHplus,tanb,alpha,VEV

      Mhl    = AMHL
      MHh    = AMHH
      MA0    = AMHA
      MHplus = AMHC
      
      tanb   = 1./RV2V1
      alpha  = -ALFAH

      VEV    = AMZ*SQRT(SN2THW)*SQRT(1.-SN2THW)/(2.*9.8696044*ALFAEM)
      
      END
C================================================================
C
C
      SUBROUTINE SUPS(Msups,theta,H,US_22)
      
      COMMON/SSSM/AMUP,AMDN,AMST,AMCH,AMBT,AMTP,AME,AMMU,AMTAU
     $,AMW,AMZ,GAMW,GAMZ,ALFAEM,SN2THW,ALFA2,ALFA3,ALQCD4
      REAL AMUP,AMDN,AMST,AMCH,AMBT,AMTP,AME,AMMU,AMTAU
     $,AMW,AMZ,GAMW,GAMZ,ALFAEM,SN2THW,ALFA2,ALFA3,ALQCD4
      SAVE /SSSM/

      COMMON/SSPAR/AMGLSS,AMULSS,AMURSS,AMDLSS,AMDRSS,AMSLSS
     $,AMSRSS,AMCLSS,AMCRSS,AMBLSS,AMBRSS,AMB1SS,AMB2SS
     $,AMTLSS,AMTRSS,AMT1SS,AMT2SS,AMELSS,AMERSS,AMMLSS,AMMRSS
     $,AMLLSS,AMLRSS,AML1SS,AML2SS,AMN1SS,AMN2SS,AMN3SS
     $,TWOM1,RV2V1,AMZ1SS,AMZ2SS,AMZ3SS,AMZ4SS,ZMIXSS(4,4)
     $,AMW1SS,AMW2SS
     $,GAMMAL,GAMMAR,AMHL,AMHH,AMHA,AMHC,ALFAH,AAT,THETAT
     $,AAB,THETAB,AAL,THETAL,AMGVSS,MTQ,MBQ,MLQ,FBMA,
     $VUQ,VDQ
      REAL AMGLSS,AMULSS,AMURSS,AMDLSS,AMDRSS,AMSLSS
     $,AMSRSS,AMCLSS,AMCRSS,AMBLSS,AMBRSS,AMB1SS,AMB2SS
     $,AMTLSS,AMTRSS,AMT1SS,AMT2SS,AMELSS,AMERSS,AMMLSS,AMMRSS
     $,AMLLSS,AMLRSS,AML1SS,AML2SS,AMN1SS,AMN2SS,AMN3SS
     $,TWOM1,RV2V1,AMZ1SS,AMZ2SS,AMZ3SS,AMZ4SS,ZMIXSS
     $,AMW1SS,AMW2SS
     $,GAMMAL,GAMMAR,AMHL,AMHH,AMHA,AMHC,ALFAH,AAT,THETAT
     $,AAB,THETAB,AAL,THETAL,AMGVSS,MTQ,MBQ,MLQ,FBMA,VUQ,VDQ
      REAL AMZISS(4)
      EQUIVALENCE (AMZISS(1),AMZ1SS)
      SAVE /SSPAR/

      REAL Msups(6),theta,H,v,v1,v2,US_22

      Msups(1) = AMULSS
      Msups(2) = AMCLSS
      Msups(3) = AMT2SS
      Msups(4) = AMURSS
      Msups(5) = AMCRSS
      Msups(6) = AMT1SS

      theta = thetat
      H     = -TWOM1
      v     = AMZ*SQRT(SN2THW)*SQRT(1.-SN2THW)/(2.*9.8696044*ALFAEM)
      v1    = v*SQRT(1./(1.+RV2V1**2))
      v2    = v1*RV2V1
C     Changed v2 <-> v1 with respect to our notation
      US_22 = AAT*AMTP*SQRT(2.)/v1
C      PRINT*, 'AAT',AAT,'YUK',AMTP*SQRT(2.)/v1
      END
C================================================================
C
C
      SUBROUTINE SDOWNS(Msdowns,theta,DS_22)
      

      COMMON/SSSM/AMUP,AMDN,AMST,AMCH,AMBT,AMTP,AME,AMMU,AMTAU
     $,AMW,AMZ,GAMW,GAMZ,ALFAEM,SN2THW,ALFA2,ALFA3,ALQCD4
      REAL AMUP,AMDN,AMST,AMCH,AMBT,AMTP,AME,AMMU,AMTAU
     $,AMW,AMZ,GAMW,GAMZ,ALFAEM,SN2THW,ALFA2,ALFA3,ALQCD4
      SAVE /SSSM/

      COMMON/SSPAR/AMGLSS,AMULSS,AMURSS,AMDLSS,AMDRSS,AMSLSS
     $,AMSRSS,AMCLSS,AMCRSS,AMBLSS,AMBRSS,AMB1SS,AMB2SS
     $,AMTLSS,AMTRSS,AMT1SS,AMT2SS,AMELSS,AMERSS,AMMLSS,AMMRSS
     $,AMLLSS,AMLRSS,AML1SS,AML2SS,AMN1SS,AMN2SS,AMN3SS
     $,TWOM1,RV2V1,AMZ1SS,AMZ2SS,AMZ3SS,AMZ4SS,ZMIXSS(4,4)
     $,AMW1SS,AMW2SS
     $,GAMMAL,GAMMAR,AMHL,AMHH,AMHA,AMHC,ALFAH,AAT,THETAT
     $,AAB,THETAB,AAL,THETAL,AMGVSS,MTQ,MBQ,MLQ,FBMA,
     $VUQ,VDQ
      REAL AMGLSS,AMULSS,AMURSS,AMDLSS,AMDRSS,AMSLSS
     $,AMSRSS,AMCLSS,AMCRSS,AMBLSS,AMBRSS,AMB1SS,AMB2SS
     $,AMTLSS,AMTRSS,AMT1SS,AMT2SS,AMELSS,AMERSS,AMMLSS,AMMRSS
     $,AMLLSS,AMLRSS,AML1SS,AML2SS,AMN1SS,AMN2SS,AMN3SS
     $,TWOM1,RV2V1,AMZ1SS,AMZ2SS,AMZ3SS,AMZ4SS,ZMIXSS
     $,AMW1SS,AMW2SS
     $,GAMMAL,GAMMAR,AMHL,AMHH,AMHA,AMHC,ALFAH,AAT,THETAT
     $,AAB,THETAB,AAL,THETAL,AMGVSS,MTQ,MBQ,MLQ,FBMA,VUQ,VDQ
      REAL AMZISS(4)
      EQUIVALENCE (AMZISS(1),AMZ1SS)
      SAVE /SSPAR/

      REAL Msdowns(6),theta,v,v1,v2,DS_22

      Msdowns(1) = AMDLSS
      Msdowns(2) = AMSLSS
      Msdowns(3) = AMB2SS
      Msdowns(4) = AMDRSS
      Msdowns(5) = AMSRSS
      Msdowns(6) = AMB1SS

      theta  = thetab
      v    = AMZ*SQRT(SN2THW)*SQRT(1.-SN2THW)/(2.*9.8696044*ALFAEM)
      v1     = v*SQRT(1./(1.+RV2V1**2))
      v2    = v1*RV2V1
C     Changed v2 <-> v1 with respect to our notation
C     there seems to be a bug in the Yuk (factor two to large)      
      DS_22  = AAB*AMBT*SQRT(2.)/(2.*v2)
C      PRINT*, 'AAB',AAB,'YUK',AMBT*SQRT(2.)/(2.*v2)
      END
C================================================================
C
C
      SUBROUTINE SLeptons(Msleptons,theta,H,LS_22)
      

      COMMON/SSSM/AMUP,AMDN,AMST,AMCH,AMBT,AMTP,AME,AMMU,AMTAU
     $,AMW,AMZ,GAMW,GAMZ,ALFAEM,SN2THW,ALFA2,ALFA3,ALQCD4
      REAL AMUP,AMDN,AMST,AMCH,AMBT,AMTP,AME,AMMU,AMTAU
     $,AMW,AMZ,GAMW,GAMZ,ALFAEM,SN2THW,ALFA2,ALFA3,ALQCD4
      SAVE /SSSM/

      COMMON/SSPAR/AMGLSS,AMULSS,AMURSS,AMDLSS,AMDRSS,AMSLSS
     $,AMSRSS,AMCLSS,AMCRSS,AMBLSS,AMBRSS,AMB1SS,AMB2SS
     $,AMTLSS,AMTRSS,AMT1SS,AMT2SS,AMELSS,AMERSS,AMMLSS,AMMRSS
     $,AMLLSS,AMLRSS,AML1SS,AML2SS,AMN1SS,AMN2SS,AMN3SS
     $,TWOM1,RV2V1,AMZ1SS,AMZ2SS,AMZ3SS,AMZ4SS,ZMIXSS(4,4)
     $,AMW1SS,AMW2SS
     $,GAMMAL,GAMMAR,AMHL,AMHH,AMHA,AMHC,ALFAH,AAT,THETAT
     $,AAB,THETAB,AAL,THETAL,AMGVSS,MTQ,MBQ,MLQ,FBMA,
     $VUQ,VDQ
      REAL AMGLSS,AMULSS,AMURSS,AMDLSS,AMDRSS,AMSLSS
     $,AMSRSS,AMCLSS,AMCRSS,AMBLSS,AMBRSS,AMB1SS,AMB2SS
     $,AMTLSS,AMTRSS,AMT1SS,AMT2SS,AMELSS,AMERSS,AMMLSS,AMMRSS
     $,AMLLSS,AMLRSS,AML1SS,AML2SS,AMN1SS,AMN2SS,AMN3SS
     $,TWOM1,RV2V1,AMZ1SS,AMZ2SS,AMZ3SS,AMZ4SS,ZMIXSS
     $,AMW1SS,AMW2SS
     $,GAMMAL,GAMMAR,AMHL,AMHH,AMHA,AMHC,ALFAH,AAT,THETAT
     $,AAB,THETAB,AAL,THETAL,AMGVSS,MTQ,MBQ,MLQ,FBMA,VUQ,VDQ
      REAL AMZISS(4)
      EQUIVALENCE (AMZISS(1),AMZ1SS)
      SAVE /SSPAR/

      REAL Msleptons(6),theta,H,v,v1,v2,LS_22

C      print *,SQRT(GSS(16)),SQRT(GSS(15))
C      print *,SQRT(GSS(21)),SQRT(GSS(20))
C      print *,SQRT(GSS(24))
C      print *,AAL
 

      Msleptons(1) = AMELSS
      Msleptons(2) = AMMLSS
      Msleptons(3) = AML2SS
      Msleptons(4) = AMERSS
      Msleptons(5) = AMMRSS
      Msleptons(6) = AML1SS

      theta = thetal
      H     = -TWOM1
C     check the sign      
      v     = AMZ*SQRT(SN2THW)*SQRT(1.-SN2THW)/(2.*9.8696044*ALFAEM)
      v1    = v*SQRT(1./(1.+RV2V1**2))
      v2    = v1*RV2V1
C     Changed v2 <-> v1 with respect to our notation
      LS_22  = AAL*AMTAU*SQRT(2.)/v2

      END
C================================================================
C
C
      SUBROUTINE GLUINO(MGL)
      

      COMMON/SSPAR/AMGLSS,AMULSS,AMURSS,AMDLSS,AMDRSS,AMSLSS
     $,AMSRSS,AMCLSS,AMCRSS,AMBLSS,AMBRSS,AMB1SS,AMB2SS
     $,AMTLSS,AMTRSS,AMT1SS,AMT2SS,AMELSS,AMERSS,AMMLSS,AMMRSS
     $,AMLLSS,AMLRSS,AML1SS,AML2SS,AMN1SS,AMN2SS,AMN3SS
     $,TWOM1,RV2V1,AMZ1SS,AMZ2SS,AMZ3SS,AMZ4SS,ZMIXSS(4,4)
     $,AMW1SS,AMW2SS
     $,GAMMAL,GAMMAR,AMHL,AMHH,AMHA,AMHC,ALFAH,AAT,THETAT
     $,AAB,THETAB,AAL,THETAL,AMGVSS,MTQ,MBQ,MLQ,FBMA,
     $VUQ,VDQ
      REAL AMGLSS,AMULSS,AMURSS,AMDLSS,AMDRSS,AMSLSS
     $,AMSRSS,AMCLSS,AMCRSS,AMBLSS,AMBRSS,AMB1SS,AMB2SS
     $,AMTLSS,AMTRSS,AMT1SS,AMT2SS,AMELSS,AMERSS,AMMLSS,AMMRSS
     $,AMLLSS,AMLRSS,AML1SS,AML2SS,AMN1SS,AMN2SS,AMN3SS
     $,TWOM1,RV2V1,AMZ1SS,AMZ2SS,AMZ3SS,AMZ4SS,ZMIXSS
     $,AMW1SS,AMW2SS
     $,GAMMAL,GAMMAR,AMHL,AMHH,AMHA,AMHC,ALFAH,AAT,THETAT
     $,AAB,THETAB,AAL,THETAL,AMGVSS,MTQ,MBQ,MLQ,FBMA,VUQ,VDQ
      REAL AMZISS(4)
      EQUIVALENCE (AMZISS(1),AMZ1SS)
      SAVE /SSPAR/

      REAL MGL

      MGL = AMGLSS
   
      END
