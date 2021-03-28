#include <wpl/group_headers_model.h>

#include <ut/assert.h>
#include <ut/test.h>

using namespace std;

namespace wpl
{
	namespace tests
	{
		namespace
		{
			class group_headers_model_impl : public group_headers_model<columns_model>
			{
			public:
				group_headers_model_impl(columns_model &underlying)
					: group_headers_model<columns_model>(underlying)
				{	}

				range_type get_group(index_type index) const throw() override
				{	return groupping(index);	}

				using group_headers_model<columns_model>::update_mapping;

			public:
				function<range_type (index_type index)> groupping;
			};



			short get_width(const columns_model &model, columns_model::index_type index)
			{
				short value;

				model.get_value(index, value);
				return value;
			}
		}

		namespace mocks
		{
			class columns_model : public wpl::columns_model
			{
			public:
				virtual index_type get_count() const throw() override
				{	return widths.size();	}

				virtual void get_value(index_type index, short &value) const override
				{	value = widths[index];	}

				virtual void set_width(index_type index, short value) override
				{
					widths[index] = value;
					invalidate(index);
				}

			public:
				vector<short> widths;
			};
		}

		begin_test_suite( GroupHeadersModelTests )
			shared_ptr<mocks::columns_model> underlying;

			init( Init )
			{
				underlying = make_shared<mocks::columns_model>();
			}


			test( GrouppedColumnWidthsAreTheSumsOfUnderlyingModelWidths )
			{
				// INIT
				short widths[] = {	12, 100, 32, 23, 1001, 97, 811,	};

				underlying->widths.assign(begin(widths), end(widths));

				// INIT / ACT
				group_headers_model_impl m(*underlying);

				// ACT / ASSERT
				assert_equal(0u, m.get_count());

				// INIT
				m.groupping = [] (size_t index) -> group_headers_model_impl::range_type {
					switch (index)
					{
					case 0:	return make_pair(0, 3);
					case 1:	return make_pair(3, 5);
					default:	return make_pair(5, 7);
					}
				};
				m.update_mapping(3);
				underlying->invalidate(columns_model::npos());

				// ACT / ASSERT
				assert_equal(3u, m.get_count());
				assert_equal(144, get_width(m, 0));
				assert_equal(1024, get_width(m, 1));
				assert_equal(908, get_width(m, 2));

				// INIT
				m.groupping = [] (size_t index) -> group_headers_model_impl::range_type {
					switch (index)
					{
					case 0:	return make_pair(0, 2);
					case 1: case 2:	return make_pair(3, 4);
					default:	return make_pair(6, 7);
					}
				};
				m.update_mapping(4);

				// ACT / ASSERT
				assert_equal(4u, m.get_count());
				assert_equal(112, get_width(m, 0));
				assert_equal(23, get_width(m, 1));
				assert_equal(23, get_width(m, 2));
				assert_equal(811, get_width(m, 3));
			}


			test( InvalidationOfAllUnderlyingColumnsResultsInInvalidationsOfAllColumns )
			{
				// INIT
				short widths[] = {	12, 100, 32, 23, 1001, 97, 811,	};
				group_headers_model_impl m(*underlying);
				vector<columns_model::index_type> invalidations;
				auto c = m.invalidate += [&] (columns_model::index_type index) {	invalidations.push_back(index);	};

				underlying->widths.assign(begin(widths), end(widths));
				m.groupping = [] (size_t /*index*/) {	return make_pair(0, 0);	};
				m.update_mapping(3);
				invalidations.clear();

				// ACT
				underlying->invalidate(columns_model::npos());

				// ASSERT
				columns_model::index_type reference1[] = {	columns_model::npos(),	};

				assert_equal(reference1, invalidations);

				// ACT
				underlying->invalidate(columns_model::npos());
				underlying->invalidate(columns_model::npos());

				// ASSERT
				columns_model::index_type reference2[] = {
					columns_model::npos(), columns_model::npos(), columns_model::npos(),
				};

				assert_equal(reference2, invalidations);
			}


			test( InvalidationOfUnderlyingColumnLeadsToInvalidationOfCorrespondingColumn )
			{
				// INIT
				short widths[] = {	1, 2, 3, 4, 5, 6, 7,	};
				group_headers_model_impl m(*underlying);
				vector<columns_model::index_type> invalidations;
				auto expected_count = 3u;
				auto c = m.invalidate += [&] (columns_model::index_type index) {
					invalidations.push_back(index);
					assert_equal(expected_count, m.get_count());
				};

				underlying->widths.assign(begin(widths), end(widths));
				m.groupping = [] (size_t index) -> group_headers_model_impl::range_type {
					switch (index)
					{
					case 0:	return make_pair(0, 1);
					case 1:	return make_pair(1, 5);
					default:	return make_pair(5, 7);
					}
				};
				m.update_mapping(3);
				invalidations.clear();

				// ACT
				underlying->invalidate(1);

				// ASSERT
				columns_model::index_type reference1[] = {	1,	};

				assert_equal(reference1, invalidations);

				// ACT
				underlying->invalidate(0);
				underlying->invalidate(2);
				underlying->invalidate(3);
				underlying->invalidate(4);
				underlying->invalidate(5);
				underlying->invalidate(6);

				// ASSERT
				columns_model::index_type reference2[] = {	1, 0, 1, 1, 1, 2, 2,	};

				assert_equal(reference2, invalidations);

				// INIT
				m.groupping = [] (size_t index) -> group_headers_model_impl::range_type {
					switch (index)
					{
					case 0:	return make_pair(0, 2);
					case 1:	return make_pair(2, 5);
					case 2:	return make_pair(5, 6);
					default:	return make_pair(6, 7);
					}
				};
				expected_count = 4;
				m.update_mapping(4);
				invalidations.clear();

				// ACT
				underlying->invalidate(0);
				underlying->invalidate(1);
				underlying->invalidate(2);
				underlying->invalidate(3);
				underlying->invalidate(4);
				underlying->invalidate(5);
				underlying->invalidate(6);

				// ASSERT
				columns_model::index_type reference3[] = {	0, 0, 1, 1, 1, 2, 3,	};

				assert_equal(4u, m.get_count());
				assert_equal(reference3, invalidations);
			}


			test( InvalidationOfAColumnFromAMissedRangeDoesNotInvalidateGroupModel )
			{
				// INIT
				short widths[] = {	1, 2, 3, 4, 5, 6, 7,	};
				group_headers_model_impl m(*underlying);

				underlying->widths.assign(begin(widths), end(widths));
				m.groupping = [] (size_t index) -> group_headers_model_impl::range_type {
					switch (index)
					{
					case 0:	return make_pair(0, 1);
					default:	return make_pair(3, 5);
					}
				};
				m.update_mapping(2);

				auto c = m.invalidate += [&] (columns_model::index_type /*index*/) {
				// ASSERT
					assert_is_false(true);
				};

				// ACT / ASSERT
				underlying->invalidate(1);
				underlying->invalidate(2);
			}


			test( UpdatingMappingInvalidatesAll )
			{
				// INIT
				short widths[] = {	1, 2, 3, 4, 5, 6, 7,	};
				group_headers_model_impl m(*underlying);
				auto invalidations = 0;
				auto expected_count = 3u;
				auto c = m.invalidate += [&] (columns_model::index_type index) {
					assert_equal(expected_count, m.get_count());
					invalidations++;
					assert_equal(columns_model::npos(), index);
				};

				underlying->widths.assign(begin(widths), end(widths));
				m.groupping = [] (size_t index) -> group_headers_model_impl::range_type {
					switch (index)
					{
					case 0:	return make_pair(0, 1);
					default:	return make_pair(1, 3);
					}
				};

				// ACT / ASSERT
				expected_count = 3;
				m.update_mapping(3);
				m.update_mapping(3);
				expected_count = 5;
				m.update_mapping(5);

				// ASSERT
				assert_equal(3, invalidations);
			}
		end_test_suite
	}
}
