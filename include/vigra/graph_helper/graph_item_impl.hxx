#ifndef VIGRA_NODE_IMPL_HXX
#define VIGRA_NODE_IMPL_HXX

/*boost*/
#include <boost/iterator/iterator_facade.hpp>

/*vigra*/
#include <vigra/algorithm.hxx>
#include <vigra/tinyvector.hxx>
#include <vigra/random_access_set.hxx>


namespace vigra{

    namespace detail{


        template<class GRAPH>
        struct NeighborNodeFilter{
            typedef typename GRAPH::Node ResultType;
            typedef typename GRAPH::NodeStorage::AdjacencyElement AdjacencyElement;

            static bool valid(
                const GRAPH & g,
                const AdjacencyElement & adj,
                const typename GRAPH::index_type ownNodeId
            ){
                return true;
            }


             static ResultType transform(
                const GRAPH & g,
                const AdjacencyElement & adj,
                const typename GRAPH::index_type ownNodeId
            ){
                return g.nodeFromId(adj.nodeId());    
            }

            static const bool IsFilter = false ; 
        };

        template<class GRAPH>
        struct IncEdgeFilter{
            typedef typename GRAPH::Edge ResultType;
            typedef typename GRAPH::NodeStorage::AdjacencyElement AdjacencyElement;

            static bool valid(
                const GRAPH & g,
                const AdjacencyElement & adj,
                const typename GRAPH::index_type ownNodeId
            ){
                return true;
            }

            static ResultType transform(
                const GRAPH & g,
                const AdjacencyElement & adj,
                const typename GRAPH::index_type ownNodeId
            ){
                return g.edgeFromId(adj.edgeId());    
            }

            static const bool IsFilter = false ; 
        };

        template<class GRAPH>
        struct BackEdgeFilter{
            typedef typename GRAPH::Edge ResultType;
            typedef typename GRAPH::NodeStorage::AdjacencyElement AdjacencyElement;
            
            static bool valid(
                const GRAPH & g,
                const AdjacencyElement & adj,
                const typename GRAPH::index_type ownNodeId
            ){
                return adj.nodeId() < ownNodeId;
            } 

            static ResultType transform(
                const GRAPH & g,
                const AdjacencyElement & adj,
                const typename GRAPH::index_type ownNodeId
            ){
                return g.edgeFromId(adj.edgeId());
            }

            static const bool IsFilter = true ; 
        };
        template<class GRAPH>
        struct IsBackOutFilter{
            typedef typename GRAPH::Arc ResultType;
            typedef typename GRAPH::NodeStorage::AdjacencyElement AdjacencyElement;
            
            static bool valid(
                const GRAPH & g,
                const AdjacencyElement & adj,
                const typename GRAPH::index_type ownNodeId
            ){
                return adj.nodeId() < ownNodeId;
            } 
            static ResultType transform(
                const GRAPH & g,
                const AdjacencyElement & adj,
                const typename GRAPH::index_type ownNodeId
            ){
                return g.direct(g.edgeFromId(adj.edgeId()) ,g.nodeFromId(ownNodeId));
            }

            static const bool IsFilter = true ; 
        };
        template<class GRAPH>
        struct IsOutFilter{
            typedef typename GRAPH::Arc ResultType;
            typedef typename GRAPH::NodeStorage::AdjacencyElement AdjacencyElement;
            
            static bool valid(
                const GRAPH & g,
                const AdjacencyElement & adj,
                const typename GRAPH::index_type ownNodeId
            ){
                return  true;
            } 
            static ResultType transform(
                const GRAPH & g,
                const AdjacencyElement & adj,
                const typename GRAPH::index_type ownNodeId
            ){
                return g.direct(g.edgeFromId(adj.edgeId()) ,g.nodeFromId(ownNodeId));
            }

            static const bool IsFilter = false ; 
        };



        template<class GRAPH>
        struct IsInFilter{
            typedef typename GRAPH::Arc ResultType;
            typedef typename GRAPH::NodeStorage::AdjacencyElement AdjacencyElement;
            
            static bool valid(
                const GRAPH & g,
                const AdjacencyElement & adj,
                const typename GRAPH::index_type ownNodeId
            ){
                return  true;//g.id(g.v(g.edgeFromId(edgeId)))==ownNodeId;
            } 
            ResultType static transform(
                const GRAPH & g,
                const AdjacencyElement & adj,
                const typename GRAPH::index_type ownNodeId
            ){
                return g.direct(g.edgeFromId(adj.edgeId()) ,g.nodeFromId(adj.nodeId()));
            }
            static const bool IsFilter = false ; 
        };


        //  This class with the filter above can create 
        //  the following lemon iterators :
        //   - IncEdgeIt
        //   - OutArcIt
        //   - 
        //
        //
        template<class GRAPH,class NODE_IMPL,class FILTER>    
        class GenericIncEdgeIt

        :  public boost::iterator_facade<
              GenericIncEdgeIt<GRAPH,NODE_IMPL,FILTER>,
              typename FILTER::ResultType const,
              boost::forward_traversal_tag
           >
        {
        public:

            typedef GRAPH Graph;
            typedef typename Graph::index_type index_type;
            typedef typename Graph::NodeIt NodeIt;
            typedef typename Graph::Edge Edge;
            typedef typename Graph::Node Node;
            typedef typename FILTER::ResultType ResultItem;
            //typedef typename GraphItemHelper<GRAPH,typename FILTER::ResultType>  ResultItem

            // default constructor
            GenericIncEdgeIt(const lemon::Invalid & invalid = lemon::INVALID)
            :   nodeImpl_(NULL),
                graph_(NULL),
                ownNodeId_(-1),
                adjIter_(),
                resultItem_(lemon::INVALID){
            }   
            // from a given node iterator
            GenericIncEdgeIt(const Graph & g , const NodeIt & nodeIt)
            :   nodeImpl_(&g.nodeImpl(*nodeIt)),
                graph_(&g),
                ownNodeId_(g.id(*nodeIt)),
                adjIter_(g.nodeImpl(*nodeIt).adjacencyBegin()),
                resultItem_(lemon::INVALID){

                if(FILTER::IsFilter){
                    while(adjIter_!=nodeImpl_->adjacencyEnd() && !FILTER::valid(*graph_,*adjIter_,ownNodeId_) ) {
                        ++adjIter_;
                    }
                }
            }

            // from a given node
            GenericIncEdgeIt(const Graph & g , const Node & node)
            :   nodeImpl_(&g.nodeImpl(node)),
                graph_(&g),
                ownNodeId_(g.id(node)),
                adjIter_(g.nodeImpl(node).adjacencyBegin()),
                resultItem_(lemon::INVALID){

                if(FILTER::IsFilter){
                    while(adjIter_!=nodeImpl_->adjacencyEnd() && !FILTER::valid(*graph_,*adjIter_,ownNodeId_) ) {
                        ++adjIter_;
                    }
                }
            }

        private:
            friend class boost::iterator_core_access;

            typedef NODE_IMPL NodeImpl;
            typedef typename NodeImpl::AdjIt AdjIt;

            bool isEnd()const{
                return  (nodeImpl_==NULL  || adjIter_==nodeImpl_->adjacencyEnd());      
            }
            bool isBegin()const{
                return (nodeImpl_!=NULL &&  adjIter_==nodeImpl_->adjacencyBegin());
            }
            bool equal(const GenericIncEdgeIt<GRAPH,NODE_IMPL,FILTER> & other)const{
                return (isEnd() && other.isEnd()) || /*(isBegin() && other.isBegin()) ||*/ ( adjIter_==other.adjIter_);
            }

            void increment(){
                ++adjIter_;
                if(FILTER::IsFilter){
                    while(adjIter_!=nodeImpl_->adjacencyEnd() && !FILTER::valid(*graph_,*adjIter_,ownNodeId_)){
                        ++adjIter_;
                    }
                }
            }

            // might no need to make this constant
            // therefore we would lose the "mutabe"
            const ResultItem & dereference()const{
                resultItem_ =  FILTER::transform(*graph_,*adjIter_,ownNodeId_);
                return resultItem_;
            }


            const NODE_IMPL * nodeImpl_;
            const GRAPH     * graph_;
            const index_type  ownNodeId_;
            AdjIt adjIter_;
            mutable ResultItem resultItem_;
        };

        /// The adjacency of a vertex consists of a vertex and a connecting edge.
        template<class T>
        class Adjacency {
        public:
            typedef T Value;

            Adjacency(const Value nodeId, const Value edgeId)
            :   nodeId_(nodeId),
                edgeId_(edgeId){

            }
            Value  nodeId() const{
                return nodeId_;
            }
            Value& nodeId(){
                return nodeId_;
            }
            Value  edgeId() const{
                return edgeId_;
            }
            Value& edgeId(){
                return edgeId_;
            }
            bool operator<(const Adjacency<Value> & other) const{
                return  nodeId_ < other.nodeId_;
            }
        private:
            Value nodeId_;
            Value edgeId_;
        };






        template<class INDEX_TYPE,bool USE_STL_SET>
        class GenericNodeImpl{

            public:
                typedef INDEX_TYPE index_type;
                typedef Adjacency<index_type>    AdjacencyElement;
                typedef std::set<AdjacencyElement >        StdSetType;
                typedef RandomAccessSet<AdjacencyElement > RandAccessSet;
                typedef typename IfBool<USE_STL_SET,StdSetType,RandAccessSet>::type SetType;

                typedef typename SetType::const_iterator AdjIt;
                


            private:
                //GenericNodeImpl();                               // non empty-construction
                //GenericNodeImpl( const GenericNodeImpl& other );      // non construction-copyable
                //GenericNodeImpl & operator=( const GenericNodeImpl& ); // non assignable
            public:

                GenericNodeImpl(const lemon::Invalid iv=lemon::INVALID)
                :  id_(-1){
                }

                GenericNodeImpl(const index_type id)
                :   id_(id){
                 }
                // query
                size_t numberOfEdges()const{return adjacency_.size();}
                size_t edgeNum()const{return adjacency_.size();}
                size_t num_edges()const{return adjacency_.size();}

                //bool hasEdgeId(const index_type edge)const{return edges_.find(edge)!=edges_.end();}

                // modification
                void  merge(const GenericNodeImpl & other){
                    adjacency_.insert(other.adjacency_.begin(),other.adjacency_.end());
                }

                
                std::pair<index_type,bool> findEdge(const index_type nodeId)const{
                    AdjIt iter = adjacency_.find(AdjacencyElement(nodeId,0));
                    if(iter==adjacency_.end()){
                        return std::pair<index_type,bool>(-1,false);
                    }
                    else{
                        return std::pair<index_type,bool>(iter->edgeId(),true);
                    }
                }



                void insert(const index_type nodeId,const index_type edgeId){
                    adjacency_.insert(AdjacencyElement(nodeId,edgeId));
                }

                AdjIt adjacencyBegin()const{
                    return adjacency_.begin();
                }
                AdjIt adjacencyEnd()const{
                    return adjacency_.end();
                }


                index_type id()const{
                    return id_;
                }
                void clear(){
                    adjacency_.clear();
                }

                void eraseFromAdjacency(const index_type nodeId){
                    // edge id does not matter?
                    adjacency_.erase(AdjacencyElement(nodeId,0));
                }

                /*
                bool eraseEdge(const size_t edgeIndex){
                    return edges_.erase(edgeIndex)==1;
                }
                void eraseAndInsert(const index_type removeEdge,const index_type insertEdge){
                    edges_.erase(removeEdge);
                    edges_.insert(insertEdge);
                }



                const EdgeIdSet & edgeIdSet()const{
                    return edges_;
                }

                void insertEdgeId(const index_type id){
                	edges_.insert(id);
                }
                */
            public:

                SetType adjacency_;
                index_type id_;
        };

        template<class INDEX_TYPE>
        class GenericEdgeImpl
        :  public vigra::TinyVector<INDEX_TYPE,3> {
                // public typedefs
            public:
                typedef INDEX_TYPE index_type;

                GenericEdgeImpl(const lemon::Invalid iv=lemon::INVALID)
                :    vigra::TinyVector<INDEX_TYPE,3>(-1){
                }

                GenericEdgeImpl(const index_type u,const index_type v, const index_type id)   
                :    vigra::TinyVector<INDEX_TYPE,3>(u,v,id){
                }
            // public methods
            public:
                index_type u()const{return this->operator[](0);}
                index_type v()const{return this->operator[](1);}
                index_type id()const{return this->operator[](2);}
            private:
        };


        template<class INDEX_TYPE>
        class GenericEdge;

        template<class INDEX_TYPE>
        class GenericArc{
        public:
            typedef INDEX_TYPE index_type;

            GenericArc(const lemon::Invalid & iv = lemon::INVALID)
            :   id_(-1),
                edgeId_(-1){

            }



            GenericArc(
                const index_type id,
                const index_type edgeId = static_cast<index_type>(-1) 
            )
            :   id_(id),
                edgeId_(edgeId){

            }
            index_type id()const{return id_;}
            index_type edgeId()const{return edgeId_;}

            operator GenericEdge<INDEX_TYPE> () const{
                return GenericEdge<INDEX_TYPE>(edgeId());
            }

            bool operator == (const GenericArc<INDEX_TYPE> & other )const{
                return id_ == other.id_;
            }
            bool operator != (const GenericArc<INDEX_TYPE> & other )const{
                return id_ != other.id_;
            }
            bool operator < (const GenericArc<INDEX_TYPE> & other )const{
                return id_ < other.id_;
            }
            bool operator > (const GenericArc<INDEX_TYPE> & other )const{
                return id_ > other.id_;
            }



        private:
            index_type id_;
            index_type edgeId_;
        };

        template<class INDEX_TYPE>
        class GenericEdge{
        public:
            typedef INDEX_TYPE index_type;

            GenericEdge(const lemon::Invalid & iv = lemon::INVALID)
            : id_(-1){

            }

            GenericEdge(const index_type id )
            : id_(id){

            }

            GenericEdge(const GenericArc<INDEX_TYPE> & arc)
            :   id_(arc.edgeId())
            {
            }

            bool operator == (const GenericEdge<INDEX_TYPE> & other )const{
                return id_ == other.id_;
            }
            bool operator != (const GenericEdge<INDEX_TYPE> & other )const{
                return id_ != other.id_;
            }
            bool operator < (const GenericEdge<INDEX_TYPE> & other )const{
                return id_ < other.id_;
            }
            bool operator > (const GenericEdge<INDEX_TYPE> & other )const{
                return id_ > other.id_;
            }


            index_type id()const{return id_;}
        private:
            index_type id_;
        };


        template<class INDEX_TYPE>
        class GenericNode{
        public:
            typedef INDEX_TYPE index_type;

            GenericNode(const lemon::Invalid & iv = lemon::INVALID)
            : id_(-1){

            }

            GenericNode(const index_type id  )
            : id_(id){
                
            }
            bool operator == (const GenericNode<INDEX_TYPE> & other )const{
                return id_ == other.id_;
            }
            bool operator != (const GenericNode<INDEX_TYPE> & other )const{
                return id_ != other.id_;
            }
            bool operator < (const GenericNode<INDEX_TYPE> & other )const{
                return id_ < other.id_;
            }
            bool operator > (const GenericNode<INDEX_TYPE> & other )const{
                return id_ > other.id_;
            }

            index_type id()const{return id_;}
        private:
            index_type id_;
        };

    }
} // end namespace vigra



#endif // VIGRA_NODE_IMPL_HXX