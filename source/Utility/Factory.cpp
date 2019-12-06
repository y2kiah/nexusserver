//	----==== FACTORY.CPP ====----

#include "Factory.h"


///// FUNCTIONS /////


template <class T>
Factory<T>::Factory() : Singleton<Factory<T>>(*this)
{

}