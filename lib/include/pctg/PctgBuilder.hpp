/*! 
 * \file PctgBuilder.hpp
 * \brief Definition of PctgBuilder class.
 * \details This file contains the definition of the class that implements the
 * construction of a paired contig.
 */

#ifndef PCTGBUILDER_HPP
#define	PCTGBUILDER_HPP

#include <stdexcept>

#include "api/BamAux.h"
#include "api/BamReader.h"

#include "types.hpp"
#include "assembly/Block.hpp"
#include "pctg/BestPctgCtgAlignment.hpp"
#include "pctg/ContigInPctgInfo.hpp"
#include "pctg/PairedContig.hpp"
#include "pctg/constraints_disattended.hpp"
#include "pool/HashContigMemPool.hpp"

#define DEFAULT_MAX_GAPS 300
#define DEFAULT_MAX_SEARCHED_ALIGNMENT 400000

#ifndef MIN_ALIGNMENT
#define MIN_ALIGNMENT 100
#endif

#ifndef MIN_HOMOLOGY
#define MIN_HOMOLOGY 85
#endif

#ifndef MIN_ALIGNMENT_QUOTIENT
#define MIN_ALIGNMENT_QUOTIENT 0.001
#endif

//! Class implementing a builder of paired contigs.
class PctgBuilder
{

private:
    const BamTools::RefVector *_masterRefVector;        //!< Reference to the id->name vector of the master contigs.
    const BamTools::RefVector *_slaveRefVector;         //!< Reference to the id->name vector of the slave contigs.
    const HashContigMemPool *_masterPool;                   //!< Reference to the master contig pool
    const HashContigMemPool *_slavePool;                    //!< Reference to the slave contig pool
    UIntType _maxAlignment;                             //!< Maximum alignment size
    UIntType _maxPctgGap;                               //!< Maximum paired contig gaps
    UIntType _maxCtgGap;                                //!< Maximum contig gaps

public:
    //! A constructor.
    /*!
     * Creates a paired contig builder.
     * \param masterPool reference to the master contigs pool
     * \param slavePool reference to the slave contigs pool
     * \param masterRefVector reference to the vector id->name of master contigs
     * \param slaveRefVector reference to the vector id->name of slave contigs
     */
    PctgBuilder(
            const HashContigMemPool *masterPool = NULL, 
            const HashContigMemPool *slavePool = NULL,
            const BamTools::RefVector *masterRefVector = NULL, 
            const BamTools::RefVector *slaveRefVector = NULL);
    
    //! Gets a master contig, given its ID.
    /*!
     * \param ctgId master contig's identifier.
     * \return the master contig with ID \c ctgId.
     */
    Contig loadMasterContig(const IdType &ctgId) const;
    
    //! Gets a slave contig, given its ID.
    /*!
     * \param ctgId slave contig's identifier.
     * \return the slave contig with ID \c ctgId.
     */
    Contig loadSlaveContig(const IdType &ctgId) const;
    
    //! Builds a paired contig with a single (master) contig.
    /*!
     * \param pctgId the paired contig's identifier
     * \param ctgId  a (master) contig identifier
     */
    PairedContig initByContig(const IdType &pctgId, const IdType &ctgId) const;
    
    //! Returns a paired contig extended using a block on the assemblies.
    /*!
     * The paired contig is expanded with the master or slave contig of the block
     * if it contains exactly one of them.
     * \param pctg a paired contig
     * \param firstBlock first block in common between the contigs
     * \param lastBlock last block in common between the contigs
     * \return the paired contig \c pctg extended using \c block.
     */
    PairedContig extendByBlock(const PairedContig &pctg, const Block &firstBlock, const Block& lastBlock) const;
    
    //! Adds the first block to a paired contig.
    /*!
     * The paired contig must be empty.
     * \param pctg a paired contig.
     * \param firstBlock first block in common between the contigs
     * \param lastBlock last block in common between the contigs
     * \return a paired contig in which the contigs of \c block may have been merged.
     * 
     * \throws std::invalid_argument if the \c pctg is not empty.
     */
    PairedContig addFirstBlockTo(PairedContig pctg, const Block &firstBlock, const Block &lastBlock) const;
    
    //! Adds the first contig to a paired contig.
    /*!
     * The contig must be one of the master assembly.
     * \param pctg a paired contig.
     * \param ctgId a master contig's ID.
     * \return a paired contig consisting of the single (master) contig \c ctgId.
     * 
     * \throws std::invalid_argument if the \c pctg is not empty.
     */
    PairedContig addFirstContigTo(const PairedContig &pctg, const IdType &ctgId) const;
    
    //! Returns a paired contig extended with a contig.
    /*!
     * \param orig a paired contig
     * \param ctg a contig
     * \param ctgInfo a ContigInPctgInfo object associated to \c ctg
     * \param pos pair of positions in \c orig and \c ctg from which to start the extension
     * \param gaps pair of gaps to fill \c ctgInfo
     * \param isMasterCtg whether \c ctg is a master or a slave contig
     * \return the extended paired contig
     */
    const PairedContig& extendPctgWithCtgFrom(
        PairedContig &orig,
        const Contig &ctg,
        ContigInPctgInfo &ctgInfo,
        const std::pair<UIntType,UIntType> &pos,
        const std::pair<UIntType,UIntType> &gaps,
        bool isMasterCtg ) const;
    
    //! Extends a paired contig adding bases to the left end.
    /*!
     * \param orig a paired contig
     * \param ctg a contig
     * \param ctgInfo a ContigInPctgInfo object associated to \c ctg
     * \param pos pair of first matching positions in \c orig and \c ctg
     * \param pctgShift number of bases to be added to the left end of \c orig
     * \param isMasterCtg whether \c ctg is a master or slave contig
     */
    const PairedContig& extendPctgWithCtgUpto(
        PairedContig &orig,
        const Contig &ctg,
        const ContigInPctgInfo &ctgInfo,
        const std::pair<UIntType,UIntType>& pos,
        const UIntType pctgShift,
        bool isMasterCtg ) const;
    
    //! Shifts a paired contig.
    /*!
     * \param orig a paired contig
     * \param shiftSize shift amount
     * \return the paired contig \c orig which has been shifted of \c shiftSize positions.
     */
    PairedContig shiftPctgOf( const PairedContig &orig, const UIntType shiftSize ) const;
    
    //! Merge a paired contig with a single contig.
    /*!
     * \param pctg a paired contig
     * \param firstBlock first block in common between the contigs
     * \param lastBlock last block in common between the contigs
     * \param mergeMasterCtg whether to merge the master or slave contig of \c block
     * \return the paired contig obtained from merging.
     */
    PairedContig mergeContig( PairedContig pctg, const Block &firstBlock, const Block &lastBlock, bool mergeMasterCtg ) const;    
    
    //! Merge a paired contig with a contig, using an alignment.
    /*!
     * \param pctg a paired contig
     * \param ctg a contig
     * \param ctgId \c ctg identifier
     * \param bestAlign best alignment of \c pctg and \c ctg
     * \param mergeMaster whether \c ctg is a master or a slave contig
     * \return the paired contig obtained from merging \c pctg and \c ctg.
     */
    const PairedContig& mergeCtgInPos(
        PairedContig &pctg,
        const Contig &ctg,
        const IdType &ctgId,
        const BestPctgCtgAlignment &bestAlign,
        bool mergeMaster ) const;
    
    //! Merge a paired contig with a master contig, using an alignment.
    /*!
     * \param pctg a paired contig
     * \param ctg a master contig
     * \param ctgId \c ctg identifier
     * \param bestAlign best alignment of \c pctg and \c ctg
     * \return the paired contig obtained from merging \c pctg and \c ctg.
     */
    const PairedContig& mergeMasterCtgInPos(
        PairedContig &pctg,
        const Contig &ctg,
        const IdType &ctgId,
        const BestPctgCtgAlignment &bestAlign ) const;
    
    //! Merge a paired contig with a slave contig, using an alignment.
    /*!
     * \param pctg a paired contig
     * \param ctg a slave contig
     * \param ctgId \c ctg identifier
     * \param bestAlign best alignment of \c pctg and \c ctg
     * \return the paired contig obtained from merging \c pctg and \c ctg.
     */
    const PairedContig& mergeSlaveCtgInPos(
        PairedContig &pctg,
        const Contig &ctg,
        const IdType &ctgId,
        const BestPctgCtgAlignment &bestAlign ) const;
    
    //! Computes the best alignment between a paired contig and a contig which may be merged.
    /*!
     * \param pctg a paired contig
     * \param pctgInfo
     * \param pctgPos position in \c pctg from which to find the alignment
     * \param ctg a contig
     * \param firstFrame first frame on \c ctg
     * \param lastFrame last frame on \c ctg
     */
    BestPctgCtgAlignment findBestAlignment(
        const PairedContig &pctg,
        const ContigInPctgInfo &pctgInfo,
        const UIntType pctgPos, Contig &ctg,
        const Frame firstFrame,
        const Frame lastFrame) const;
};

#endif	/* PCTGBUILDER_HPP */

