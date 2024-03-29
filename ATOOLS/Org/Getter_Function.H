#ifndef ATOOLS_Org_Getter_Function_H
#define ATOOLS_Org_Getter_Function_H

#include <map>
#include <vector>
#include <string>

namespace ATOOLS {

  class String_Sort;

  template <class ObjectType,class ParameterType,
	    class SortCriterion=std::less<std::string> >
  class Getter_Function {
  public:

    typedef ObjectType Object_Type;
    typedef SortCriterion Sort_Criterion;
    typedef std::vector<const Getter_Function *> Getter_List;
    typedef ParameterType Parameter_Type;
    typedef typename std::map<const std::string,Getter_Function *const,
			      Sort_Criterion> String_Getter_Map;

  private:

    static String_Getter_Map *s_getters;

    static bool s_exactmatch;
    bool m_display;

  protected:

    virtual Object_Type * 
    operator()(const Parameter_Type &parameters) const;

    virtual void PrintInfo(std::ostream &str,const size_t width) const;

  public:
    
    // constructor
    Getter_Function(const std::string &name);

    // destructor
    virtual ~Getter_Function();

    // member functions
    static void PrintGetterInfo(std::ostream &str,const size_t width,
                                const std::string &indent="   ",
                                const std::string &separator=" ",
                                const std::string &lineend="\n",
                                const std::string &replacefrom="",
                                const std::string &replaceto="");

    Object_Type *GetObject(const Parameter_Type &parameters) const;

    static Object_Type *GetObject(const std::string &name,
				  const Parameter_Type &parameters);
    static Getter_List GetGetters(const std::string &name="");

    // inline functions
    inline static void SetExactMatch(const bool exactmatch)
    { s_exactmatch=exactmatch; }
    inline void SetDisplay(const bool display)
    { m_display=display; }
    
    inline static bool ExactMatch() { return s_exactmatch; }
  
  };// end of class Getter_Function

  template <class ObjectType,class ParameterType,class TagType,
	    class SortCriterion=std::less<std::string> >
  class Getter {};

}// end of namespace ATOOLS

#define DECLARE_GETTER_BASE(NAME,TAG,OBJECT,PARAMETER,SORT)		\
  									\
  namespace ATOOLS {							\
    template <> class Getter<OBJECT,PARAMETER,NAME,SORT>:		\
      public Getter_Function<OBJECT,PARAMETER,SORT> {			\
    private:								\
      static Getter<OBJECT,PARAMETER,NAME,SORT> s_initializer;		\
    protected:								\
      void PrintInfo(std::ostream &str,const size_t width) const;	\
      Object_Type *operator()(const Parameter_Type &parameters) const;	\
    public:								\
      Getter(const bool &d=true):		\
	Getter_Function<OBJECT,PARAMETER,SORT>(TAG)			\
	{ SetDisplay(d); }						\
    };									\
  }

#define DECLARE_ND_GETTER(NAME,TAG,OBJECT,PARAMETER,DISP)		\
  DECLARE_GETTER_BASE(NAME,TAG,OBJECT,PARAMETER,std::less<std::string>)	\
  ATOOLS::Getter<OBJECT,PARAMETER,NAME>					\
  ATOOLS::Getter<OBJECT,PARAMETER,NAME>::s_initializer(DISP)				 

#define DECLARE_GETTER(NAME,TAG,OBJECT,PARAMETER)			\
  DECLARE_ND_GETTER(NAME,TAG,OBJECT,PARAMETER,true)

#define DECLARE_NI_GETTER(NAME,OBJECT,PARAMETER,SORT)			\
  									\
  namespace ATOOLS {							\
    template <> class Getter<OBJECT,PARAMETER,NAME,SORT>:		\
      public ATOOLS::Getter_Function<OBJECT,PARAMETER,SORT> {		\
    protected:								\
      void PrintInfo(std::ostream &str,const size_t width) const;	\
      Object_Type *operator()(const Parameter_Type &parameters) const;	\
    public:								\
      Getter(const std::string &name,const bool d=true):			\
	Getter_Function<OBJECT,PARAMETER,SORT>(name)			\
	{ SetDisplay(d); }						\
    };									\
  }

#endif
