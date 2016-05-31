#pragma once

#include "types.hpp"

#include <iostream>

template< typename ID, typename F, typename L, typename H >
inline std::tuple< volume_ptr<ID>, std::vector<std::size_t> >
watershed( const affinity_graph_ptr<F>& aff_ptr, const L& lowv, const H& highv )
{
    using affinity_t = F;
    using id_t       = ID;
    using traits     = watershed_traits<id_t>;

    affinity_t low  = static_cast<affinity_t>(lowv);
    affinity_t high = static_cast<affinity_t>(highv);

    std::ptrdiff_t xdim = aff_ptr->shape()[0];
    std::ptrdiff_t ydim = aff_ptr->shape()[1];
    std::ptrdiff_t zdim = aff_ptr->shape()[2];

    std::ptrdiff_t size = xdim * ydim * zdim;

    std::tuple< volume_ptr<id_t>, std::vector<std::size_t> > result
        ( volume_ptr<id_t>( new volume<id_t>(boost::extents[xdim][ydim][zdim],
                                           boost::fortran_storage_order())),
          std::vector<std::size_t>(1) );

    auto& counts = std::get<1>(result);
    counts[0] = 0;

    affinity_graph<F>& aff = *aff_ptr;
    volume<id_t>&      seg = *std::get<0>(result);

    id_t* seg_raw = seg.data();
    int num_deleted = 0;
    for ( std::ptrdiff_t z = 0; z < zdim; ++z )
        for ( std::ptrdiff_t y = 0; y < ydim; ++y )
            for ( std::ptrdiff_t x = 0; x < xdim; ++x )
            {
                id_t& id = seg[x][y][z] = 0;

                F negx = (x>0) ? aff[x][y][z][0] : low;
                F negy = (y>0) ? aff[x][y][z][1] : low;
                F negz = (z>0) ? aff[x][y][z][2] : low;
                F posx = (x<(xdim-1)) ? aff[x+1][y][z][0] : low;
                F posy = (y<(ydim-1)) ? aff[x][y+1][z][1] : low;
                F posz = (z<(zdim-1)) ? aff[x][y][z+1][2] : low;

                F m = std::max({negx,negy,negz,posx,posy,posz});

                if ( m > low )
                {
                    if ( negx == m || negx >= high ) { id |= 0x01;num_deleted++; }
                    if ( negy == m || negy >= high ) { id |= 0x02;num_deleted++; }
                    if ( negz == m || negz >= high ) { id |= 0x04;num_deleted++; }
                    if ( posx == m || posx >= high ) { id |= 0x08;num_deleted++; }
                    if ( posy == m || posy >= high ) { id |= 0x10;num_deleted++; }
                    if ( posz == m || posz >= high ) { id |= 0x20;num_deleted++; }
                }
            }
    std::cout << "num_deleted " << num_deleted << std::endl;

    const std::ptrdiff_t dir[6] = { -1, -xdim, -xdim*ydim, 1, xdim, xdim*ydim };
    const id_t dirmask[6]  = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20 };
    const id_t idirmask[6] = { 0x08, 0x10, 0x20, 0x01, 0x02, 0x04 };

    // get plato corners
    int num_corners = 0;
    int num_bi = 0;
    int num_out = 0;

    std::vector<std::ptrdiff_t> bfs;

    for ( std::ptrdiff_t idx = 0; idx < size; ++idx )
    {
        for ( std::ptrdiff_t d = 0; d < 6; ++d )
        {
            if ( seg_raw[idx] & dirmask[d] ) // not connected in d dir
            {
                if ( !(seg_raw[idx+dir[d]] & idirmask[d]) ) // connected
                {
                    seg_raw[idx] |= 0x40;
                    bfs.push_back(idx);
                    d = 6; // break;
                    num_corners++;
                }
            }
        }
    }
    std::cout << "num corners: " << bfs.size() << std::endl;

    int num_corners_all = 0;
    for ( std::ptrdiff_t idx = 0; idx < size; ++idx )
    {
        for ( std::ptrdiff_t d = 0; d < 6; ++d )
        {
            if ( seg_raw[idx] & dirmask[d] ) // not connected in d dir
            {
                if ( !(seg_raw[idx+dir[d]] & idirmask[d]) ) // connected
                {
                    num_corners_all++;
                }
            }
        }
    }
    std::cout << "num corners all: " << num_corners_all << std::endl;
    for(int i=0;i<bfs.size();i++){
        //std::cout << "\t" << i << ": " << bfs[i] << std::endl;
    }
    // divide the plateaus

    std::size_t bfs_index = 0;
    int num_pops = 0;
    int num_pushes = 0;
    while ( bfs_index < bfs.size() )
    {
        std::ptrdiff_t idx = bfs[bfs_index];
        num_pops++;
        id_t to_set = 0;

        for ( std::ptrdiff_t d = 0; d < 6; ++d )
        {
            if ( seg_raw[idx] & dirmask[d] )
            {
                if ( seg_raw[idx+dir[d]] & idirmask[d] )
                {
                    if ( !( seg_raw[idx+dir[d]] & 0x40 ) )
                    {
                        bfs.push_back(idx+dir[d]);
                        num_pushes++;
                        seg_raw[idx+dir[d]] |= 0x40;
                    }
                }
                else
                {
                    to_set = dirmask[d];
                }
            }
        }
        seg_raw[idx] = to_set;
        ++bfs_index;
    }
    std::cout << "num_pops: " <<num_pops << std::endl;
    std::cout << "num_pushes: " << num_pushes << std::endl;

    bfs.clear();

    // main watershed logic

    id_t next_id = 1;

    for ( std::ptrdiff_t idx = 0; idx < size; ++idx )
    {
        if ( seg_raw[idx] == 0 )
        {
            seg_raw[idx] |= traits::high_bit;
            ++counts[0];
        }

        if ( !( seg_raw[idx] & traits::high_bit ) && seg_raw[idx] )
        {
            bfs.push_back(idx);
            bfs_index = 0;
            seg_raw[idx] |= 0x40;

            while ( bfs_index < bfs.size() )
            {
                std::ptrdiff_t me = bfs[bfs_index];

                for ( std::ptrdiff_t d = 0; d < 6; ++d )
                {
                    if ( seg_raw[me] & dirmask[d] )
                    {
                        std::ptrdiff_t him = me + dir[d];
                        if ( seg_raw[him] & traits::high_bit )
                        {
                            counts[ seg_raw[him] & ~traits::high_bit ]
                                += bfs.size();

                            for ( auto& it: bfs )
                            {
                                seg_raw[it] = seg_raw[him];
                            }

                            bfs.clear();
                            d = 6; // break
                        }
                        else if ( !( seg_raw[him] & 0x40 ) )
                        {
                            seg_raw[him] |= 0x40;
                            bfs.push_back( him );

                        }
                    }
                }
                ++bfs_index;
            }

            if ( bfs.size() )
            {
                counts.push_back( bfs.size() );
                for ( auto& it: bfs )
                {
                    seg_raw[it] = traits::high_bit | next_id;
                }
                ++next_id;
                bfs.clear();
            }
        }
    }

    std::cout << "found: " << (next_id-1) << " components\n";

    for ( std::ptrdiff_t idx = 0; idx < size; ++idx )
    {
        seg_raw[idx] &= traits::mask;
    }

    return result;
}
