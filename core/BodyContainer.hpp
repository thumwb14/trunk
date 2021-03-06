// 2004 © Olivier Galizzi <olivier.galizzi@imag.fr>
// 2004 © Janek Kozicki <cosurgi@berlios.de>
// 2010 © Václav Šmilauer <eudoxos@arcig.cz>

#pragma once

#include <lib/serialization/Serializable.hpp>
#include <boost/tuple/tuple.hpp>

class Body;
class InteractionContainer;

#if YADE_OPENMP
	#define YADE_PARALLEL_FOREACH_BODY_BEGIN(b_,bodies) const Body::id_t _sz(bodies->size()); _Pragma("omp parallel for") for(Body::id_t _id=0; _id<_sz; _id++){ if(!(*bodies)[_id])  continue; b_((*bodies)[_id]);
	#define YADE_PARALLEL_FOREACH_BODY_END() }
#else
	#define YADE_PARALLEL_FOREACH_BODY_BEGIN(b,bodies) FOREACH(b,*(bodies)){
	#define YADE_PARALLEL_FOREACH_BODY_END() }
#endif

/*
Container of bodies implemented as flat std::vector. It handles body removal and
intelligently reallocates free ids for newly added ones.
The nested iterators and the specialized FOREACH_BODY macros above will silently skip null body pointers which may exist after removal. The null pointers can still be accessed via the [] operator. 

Any alternative implementation should use the same API.
*/
class BodyContainer: public Serializable{
	private:
		using ContainerT = std::vector<shared_ptr<Body> > ;
		using MemberMap = std::map<Body::id_t,Se3r> ;
// 		ContainerT body;
	public:
		friend class InteractionContainer;  // accesses the body vector directly
		
		//An iterator that will automatically jump slots with null bodies
		class smart_iterator : public ContainerT::iterator {
			public:
			ContainerT::iterator end;
			smart_iterator& operator++() {
				ContainerT::iterator::operator++();
				while (!(this->operator*()) && end!=(*this)) ContainerT::iterator::operator++();
				return *this;}
			smart_iterator operator++(int) {smart_iterator temp(*this); operator++(); return temp;}
			smart_iterator& operator=(const ContainerT::iterator& rhs) {ContainerT::iterator::operator=(rhs); return *this;}
			smart_iterator& operator=(const smart_iterator& rhs) {ContainerT::iterator::operator=(rhs); end=rhs.end; return *this;}
			smart_iterator() {}
			smart_iterator(const ContainerT::iterator& source) {(*this)=source;}
			smart_iterator(const smart_iterator& source) {(*this)=source; end=source.end;}
		};
		using iterator = smart_iterator ;
		using const_iterator = const smart_iterator ;

// 		BodyContainer() {};
		virtual ~BodyContainer() {};
		Body::id_t insert(shared_ptr<Body>); // => body.push_back()
		Body::id_t insertAtId(shared_ptr<Body> b, Body::id_t candidate);  // => body[candidate]=...
			
		void clear();
		iterator begin() {
			iterator temp(body.begin());
			temp.end=body.end();
			return (body.begin()==body.end() || *temp)?temp:++temp;}
		iterator end() {
			iterator temp(body.end());
			temp.end=body.end();
			return temp;
		}

		const size_t size() const { return body.size(); }
		shared_ptr<Body>& operator[](unsigned int id){ return body[id];}
		const shared_ptr<Body>& operator[](unsigned int id) const { return body[id]; }

		const bool exists(Body::id_t id) const {
			return ((id>=0) && ((size_t)id<body.size()) && ((bool)body[id]));
		}
		bool erase(Body::id_t id, bool eraseClumpMembers);
		
		YADE_CLASS_BASE_DOC_ATTRS(BodyContainer,Serializable,"Standard body container for a scene",
		((ContainerT,body,,,"The underlying vector<shared_ptr<Body> >"))
		)

	DECLARE_LOGGER;
};
REGISTER_SERIALIZABLE(BodyContainer);
